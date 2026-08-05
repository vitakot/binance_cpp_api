/**
Binance Futures WebSocket Stream manager

Licensed under the MIT License <http://opensource.org/licenses/MIT>.
SPDX-License-Identifier: MIT
Copyright (c) 2022 Vitezslav Kot <vitezslav.kot@stonky.cz>, Stonky s.r.o.
*/

#include <stonky/binance/binance_futures_rest_client.h>
#include "stonky/binance/binance_ws_stream_manager.h"
#include "stonky/binance/binance_futures_ws_client.h"
#include <boost/algorithm/string/case_conv.hpp>
#include <atomic>
#include <chrono>
#include <mutex>
#include <thread>
#include "stonky/utils/magic_enum_wrapper.hpp"


using namespace std::chrono_literals;

namespace stonky::binance::futures {
/// Granularity of the polling loops of the read operations
static constexpr auto READ_POLL_INTERVAL = 3ms;

struct WSStreamManager::P {
    std::unique_ptr<WebSocketClient> wsClient;
    std::atomic<int> timeout{5};
    std::atomic<int> maxTickAge{60};
    std::string listenKey;
    mutable std::recursive_mutex tickerLocker;
    mutable std::recursive_mutex candlestickLocker;
    std::map<std::string, EventTickPrice> tickPrices;
    std::map<std::string, std::map<CandleInterval, EventCandlestick> > candlesticks;
    std::map<std::string, std::map<CandleInterval, EventCandlestick> > candlesticksHistoric;
    std::weak_ptr<RESTClient> restClient;
    onLogMessage logMessageCB;

    explicit P(const std::weak_ptr<RESTClient> &restClient) {
        wsClient = std::make_unique<WebSocketClient>();
        this->restClient = restClient;
    }

    /// Binance sends symbols in upper case, normalize the map keys so that a lower case query still hits the entry
    static std::string normalizeSymbol(const std::string &pair) {
        return boost::algorithm::to_upper_copy(pair);
    }

    static std::int64_t nowMs() {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
    }

    /// The logger callback is optional, never call it unconditionally
    void log(const LogSeverity severity, const std::string &message) const {
        if (logMessageCB) {
            logMessageCB(severity, message);
        }
    }

    [[nodiscard]] bool isStale(const EventTickPrice &tickPrice) const {
        const auto maxAge = maxTickAge.load();

        if (maxAge <= 0) {
            return false;
        }

        const auto eventTime = std::max(tickPrice.E, tickPrice.T);
        return eventTime <= 0 || nowMs() - eventTime > static_cast<std::int64_t>(maxAge) * 1000;
    }

    /**
     * Fetch the current best bid/ask over REST and store it in the tick price cache. Used both for seeding a fresh
     * subscription and for refreshing an entry that went stale because the stream is silent or dead.
     * @param symbol normalized (upper case) symbol
     * @return the stored EventTickPrice or bad option when the snapshot could not be obtained
     */
    std::optional<EventTickPrice> refreshTickPriceFromREST(const std::string &symbol) {
        const auto client = restClient.lock();

        if (!client) {
            return {};
        }

        BookTickerPrice snapshot;

        try {
            snapshot = client->getBookTickerPrice(symbol);
        } catch (std::exception &e) {
            log(LogSeverity::Error, fmt::format("{}: REST book ticker snapshot for {} failed: {}", MAKE_FILELINE,
                                                symbol, e.what()));
            return {};
        }

        if (snapshot.askPrice <= 0.0 || snapshot.bidPrice <= 0.0) {
            log(LogSeverity::Warning, fmt::format("{}: REST book ticker snapshot for {} has no valid bid/ask",
                                                  MAKE_FILELINE, symbol));
            return {};
        }

        /// The snapshot has just been received, stamp it with the local time so that the freshness check does not
        /// depend on the clock difference between the server and the machine
        const auto receiveTime = nowMs();

        std::lock_guard lk(tickerLocker);
        const auto it = tickPrices.find(symbol);

        if (it == tickPrices.end()) {
            EventTickPrice tickPrice;
            tickPrice.s = symbol;
            tickPrice.a = snapshot.askPrice;
            tickPrice.b = snapshot.bidPrice;
            tickPrice.A = snapshot.askQty;
            tickPrice.B = snapshot.bidQty;
            tickPrice.E = receiveTime;
            tickPrice.T = snapshot.time;
            return tickPrices.insert({symbol, tickPrice}).first->second;
        }

        /// Keep the accumulated volume counters, only the prices and the timestamps are refreshed
        it->second.a = snapshot.askPrice;
        it->second.b = snapshot.bidPrice;
        it->second.E = receiveTime;
        it->second.T = snapshot.time;
        return it->second;
    }
};

WSStreamManager::WSStreamManager(const std::weak_ptr<RESTClient> &restClient) : m_p(
    std::make_unique<P>(restClient)) {
}

WSStreamManager::~WSStreamManager() {
    /// Release the readers first, they poll on the timeout value
    m_p->timeout = 0;
    m_p->wsClient.reset();
}

void WSStreamManager::subscribeBookTickerStream(const std::string &pair, bool) const {
    if (m_p->wsClient->findStream(WebSocketClient::composeStreamName(pair, "bookTicker"))) {
        return;
    }

    m_p->log(LogSeverity::Info,
             fmt::format("subscribing: {}", WebSocketClient::composeStreamName(pair, "bookTicker")));

    m_p->wsClient->bookTicker(pair, [this](const nlohmann::json &msg) {
        std::lock_guard lk(m_p->tickerLocker);
        EventTickPrice eventMsg;
        eventMsg.fromJson(msg);
        eventMsg.s = P::normalizeSymbol(eventMsg.s);

        try {
            if (const auto it = m_p->tickPrices.find(eventMsg.s); it != m_p->tickPrices.end()) {
                it->second.a = eventMsg.a;
                it->second.b = eventMsg.b;
                it->second.u = eventMsg.u;
                it->second.T = eventMsg.T;
                it->second.E = eventMsg.E;
                it->second.e = eventMsg.e;

                /// Accumulate volume between read outs, otherwise the volume information would be lost!
                it->second.A += eventMsg.A;
                it->second.B += eventMsg.B;
            } else {
                m_p->tickPrices.insert({eventMsg.s, eventMsg});
            }
        } catch (nlohmann::json::exception &e) {
            m_p->log(LogSeverity::Error, fmt::format("{}: {}", MAKE_FILELINE, e.what()));
        }
    });

    m_p->wsClient->run();

    /// The stream only pushes when the best bid/ask changes, for an illiquid symbol that can take minutes. Seed the
    /// cache from the REST snapshot so that the price is available right away.
    m_p->refreshTickPriceFromREST(P::normalizeSymbol(pair));
}

void WSStreamManager::subscribeCandlestickStream(const std::string &pair, const CandleInterval interval, bool) const {
    std::string channel("kline");
    channel.append(magic_enum::enum_name(interval));

    if (m_p->wsClient->findStream(WebSocketClient::composeStreamName(pair, channel))) {
        return;
    }

    m_p->log(LogSeverity::Info, fmt::format("subscribing: {}", WebSocketClient::composeStreamName(pair, channel)));

    m_p->wsClient->candlestick(pair, interval, [this](const nlohmann::json &msg) {
        std::lock_guard lk(m_p->candlestickLocker);
        EventCandlestick eventMsg;
        eventMsg.fromJson(msg);
        eventMsg.s = P::normalizeSymbol(eventMsg.s);

        try {
            std::optional<EventCandlestick> previousCandle;
            /// Insert new candle
            {
                auto it = m_p->candlesticks.find(eventMsg.s);

                if (it == m_p->candlesticks.end()) {
                    m_p->candlesticks.insert({eventMsg.s, {}});
                }

                it = m_p->candlesticks.find(eventMsg.s);

                if (const auto itInterval = it->second.find(eventMsg.k.i);
                    itInterval != it->second.end() && itInterval->second.k.t != eventMsg.k.t) {
                    previousCandle = itInterval->second;
                }

                it->second.insert_or_assign(eventMsg.k.i, eventMsg);
            }

            /// Update historic candle
            {
                if (previousCandle) {
                    auto it = m_p->candlesticksHistoric.find(eventMsg.s);

                    if (it == m_p->candlesticksHistoric.end()) {
                        m_p->candlesticksHistoric.insert({eventMsg.s, {}});
                    }

                    it = m_p->candlesticksHistoric.find(eventMsg.s);
                    it->second.insert_or_assign(previousCandle->k.i,
                                                *previousCandle);
                }
            }
        } catch (nlohmann::json::exception &e) {
            m_p->log(LogSeverity::Error, fmt::format("{}: {}", MAKE_FILELINE, e.what()));
        }
    });

    m_p->wsClient->run();
}

void WSStreamManager::setTimeout(const int seconds) const {
    m_p->timeout = seconds;
}

int WSStreamManager::timeout() const {
    return m_p->timeout;
}

void WSStreamManager::setMaxTickAge(const int seconds) const {
    m_p->maxTickAge = seconds;
}

int WSStreamManager::maxTickAge() const {
    return m_p->maxTickAge;
}

void WSStreamManager::setLoggerCallback(const onLogMessage &onLogMessageCB) const {
    m_p->logMessageCB = onLogMessageCB;
    m_p->wsClient->setLoggerCallback(onLogMessageCB);
}

std::optional<EventTickPrice> WSStreamManager::readEventTickPrice(const std::string &pair,
                                                                  const bool consumeEvent) const {
    const auto symbol = P::normalizeSymbol(pair);
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(m_p->timeout.load());

    for (;;) {
        if (m_p->timeout == 0) {
            /// No need to wait when destroying object
            return {};
        }

        {
            std::lock_guard lk(m_p->tickerLocker);

            if (const auto it = m_p->tickPrices.find(symbol); it != m_p->tickPrices.end()) {
                if (!m_p->isStale(it->second)) {
                    auto retVal = it->second;
                    /// Zero volume counters - prepare for accumulating
                    it->second.A = 0.0;
                    it->second.B = 0.0;

                    if (consumeEvent) {
                        m_p->tickPrices.erase(it);
                    }

                    return retVal;
                }

                /// The cached tick is too old - the stream is silent or dead. Do not serve a stale price, go to REST.
                break;
            }
        }

        if (std::chrono::steady_clock::now() >= deadline) {
            break;
        }

        std::this_thread::sleep_for(READ_POLL_INTERVAL);
    }

    /// Nothing usable arrived over the stream, fall back to the REST snapshot. This is the normal path for illiquid
    /// symbols whose best bid/ask does not change for minutes.
    const auto snapshot = m_p->refreshTickPriceFromREST(symbol);

    if (snapshot) {
        std::lock_guard lk(m_p->tickerLocker);

        if (consumeEvent) {
            m_p->tickPrices.erase(symbol);
        } else if (const auto it = m_p->tickPrices.find(symbol); it != m_p->tickPrices.end()) {
            /// Zero volume counters - the accumulated volume is being returned to the caller now
            it->second.A = 0.0;
            it->second.B = 0.0;
        }
    }

    return snapshot;
}

std::optional<EventCandlestick>
WSStreamManager::readEventCandlestick(const std::string &pair, const CandleInterval interval,
                                      const bool previous) const {
    const auto symbol = P::normalizeSymbol(pair);
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(m_p->timeout.load());

    for (;;) {
        if (m_p->timeout == 0) {
            /// No need to wait when destroying object
            break;
        }

        {
            std::lock_guard lk(m_p->candlestickLocker);
            const auto &source = previous ? m_p->candlesticksHistoric : m_p->candlesticks;

            if (const auto it = source.find(symbol); it != source.end()) {
                if (const auto itCandle = it->second.find(interval); itCandle != it->second.end()) {
                    return itCandle->second;
                }
            }
        }

        if (std::chrono::steady_clock::now() >= deadline) {
            break;
        }

        std::this_thread::sleep_for(READ_POLL_INTERVAL);
    }

    return {};
}
}
