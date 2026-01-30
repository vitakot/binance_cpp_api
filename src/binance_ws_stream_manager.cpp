/**
Binance Futures WebSocket Stream manager

Licensed under the MIT License <http://opensource.org/licenses/MIT>.
SPDX-License-Identifier: MIT
Copyright (c) 2022 Vitezslav Kot <vitezslav.kot@gmail.com>.
*/

#include <vk/binance/binance_futures_rest_client.h>
#include "vk/binance/binance_ws_stream_manager.h"
#include "vk/binance/binance_futures_ws_client.h"
#include <mutex>
#include <thread>
#include "vk/utils/magic_enum_wrapper.hpp"


using namespace std::chrono_literals;

namespace vk::binance::futures {
struct WSStreamManager::P {
    std::unique_ptr<WebSocketClient> wsClient;
    int timeout{5};
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
};

WSStreamManager::WSStreamManager(const std::weak_ptr<RESTClient> &restClient) : m_p(
    std::make_unique<P>(restClient)) {
}

WSStreamManager::~WSStreamManager() {
    m_p->wsClient.reset();
    m_p->timeout = 0;
}

void WSStreamManager::subscribeBookTickerStream(const std::string &pair, bool) const {
    if (m_p->wsClient->findStream(WebSocketClient::composeStreamName(pair, "bookTicker"))) {
        return;
    }

    if (m_p->logMessageCB) {
        const auto msgString = fmt::format("subscribing: {}", WebSocketClient::composeStreamName(pair, "bookTicker"));
        m_p->logMessageCB(LogSeverity::Info, msgString);
    }

    m_p->wsClient->bookTicker(pair, [&](const nlohmann::json &msg) {
        std::lock_guard lk(m_p->tickerLocker);
        EventTickPrice eventMsg;
        eventMsg.fromJson(msg);

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
            m_p->logMessageCB(LogSeverity::Error, fmt::format("{}: {}", MAKE_FILELINE, e.what()));
        }
    });

    m_p->wsClient->run();
}

void WSStreamManager::subscribeCandlestickStream(const std::string &pair, const CandleInterval interval, bool) const {
    std::string channel("kline");
    channel.append(magic_enum::enum_name(interval));

    if (m_p->wsClient->findStream(WebSocketClient::composeStreamName(pair, channel))) {
        return;
    }

    if (m_p->logMessageCB) {
        const auto msgString = fmt::format("subscribing: {}", WebSocketClient::composeStreamName(pair, channel));
        m_p->logMessageCB(LogSeverity::Info, msgString);
    }

    m_p->wsClient->candlestick(pair, interval, [&](const nlohmann::json &msg) {
        std::lock_guard lk(m_p->candlestickLocker);
        EventCandlestick eventMsg;
        eventMsg.fromJson(msg);

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
            m_p->logMessageCB(LogSeverity::Error, fmt::format("{}: {}", MAKE_FILELINE, e.what()));
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

void WSStreamManager::setLoggerCallback(const onLogMessage &onLogMessageCB) const {
    m_p->logMessageCB = onLogMessageCB;
    m_p->wsClient->setLoggerCallback(onLogMessageCB);
}

std::optional<EventTickPrice> WSStreamManager::readEventTickPrice(const std::string &pair,
                                                                  const bool consumeEvent) const {
    int numTries = 0;
    const int maxNumTries = static_cast<int>(m_p->timeout / 0.01);

    while (numTries <= maxNumTries) {
        if (m_p->timeout == 0) {
            /// No need to wait when destroying object
            break;
        }

        m_p->tickerLocker.lock();

        if (const auto it = m_p->tickPrices.find(pair); it != m_p->tickPrices.end()) {
            auto retVal = it->second;
            /// Zero volume counters - prepare for accumulating
            it->second.A = 0.0;
            it->second.B = 0.0;

            if (consumeEvent) {
                m_p->tickPrices.erase(it);
            }

            m_p->tickerLocker.unlock();
            return retVal;
        }
        m_p->tickerLocker.unlock();
        numTries++;
        std::this_thread::sleep_for(3ms);
    }

    return {};
}

std::optional<EventCandlestick>
WSStreamManager::readEventCandlestick(const std::string &pair, const CandleInterval interval,
                                      const bool previous) const {
    int numTries = 0;
    const int maxNumTries = static_cast<int>(m_p->timeout / 0.01);

    while (numTries <= maxNumTries) {
        if (m_p->timeout == 0) {
            /// No need to wait when destroying object
            break;
        }

        if (previous) {
            m_p->candlestickLocker.lock();

            if (const auto it = m_p->candlesticksHistoric.find(pair); it != m_p->candlesticksHistoric.end()) {
                if (const auto itCandle = it->second.find(interval); itCandle != it->second.end()) {
                    auto retVal = itCandle->second;
                    m_p->candlestickLocker.unlock();
                    return retVal;
                }
            }
            m_p->candlestickLocker.unlock();
        } else {
            m_p->candlestickLocker.lock();

            if (const auto it = m_p->candlesticks.find(pair); it != m_p->candlesticks.end()) {
                if (const auto itCandle = it->second.find(interval); itCandle != it->second.end()) {
                    auto retVal = itCandle->second;
                    m_p->candlestickLocker.unlock();
                    return retVal;
                }
            }
            m_p->candlestickLocker.unlock();
        }
        numTries++;
        std::this_thread::sleep_for(3ms);
    }

    return {};
}
}
