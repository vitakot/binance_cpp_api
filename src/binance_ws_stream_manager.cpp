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
#include "magic_enum/magic_enum.hpp"


using namespace std::chrono_literals;

namespace vk::binance::futures {
struct WSStreamManager::P {
    std::unique_ptr<WebSocketClient> m_wsClient;
    int m_timeout{5};
    std::string m_listenKey;
    mutable std::recursive_mutex m_tickerLocker;
    mutable std::recursive_mutex m_candlestickLocker;
    std::map<std::string, EventTickPrice> m_tickPrices;
    std::map<std::string, std::map<CandleInterval, EventCandlestick> > m_candlesticks;
    std::map<std::string, std::map<CandleInterval, EventCandlestick> > m_candlesticksHistoric;
    std::weak_ptr<RESTClient> m_restClient;
    onLogMessage m_logMessageCB;

    explicit P(const std::weak_ptr<RESTClient> &restClient) {
        m_wsClient = std::make_unique<WebSocketClient>();
        m_restClient = restClient;
    }
};

WSStreamManager::WSStreamManager(const std::weak_ptr<RESTClient> &restClient) : m_p(
    std::make_unique<P>(restClient)) {
}

WSStreamManager::~WSStreamManager() {
    m_p->m_wsClient.reset();
    m_p->m_timeout = 0;
}

void WSStreamManager::subscribeBookTickerStream(const std::string &pair, bool) const {
    if (m_p->m_wsClient->findStream(WebSocketClient::composeStreamName(pair, "bookTicker"))) {
        return;
    }

    if (m_p->m_logMessageCB) {
        const auto msgString = fmt::format("subscribing: {}", WebSocketClient::composeStreamName(pair, "bookTicker"));
        m_p->m_logMessageCB(LogSeverity::Info, msgString);
    }

    m_p->m_wsClient->bookTicker(pair, [&](const nlohmann::json &msg) {
        std::lock_guard lk(m_p->m_tickerLocker);
        EventTickPrice eventMsg;
        eventMsg.fromJson(msg);

        try {
            if (const auto it = m_p->m_tickPrices.find(eventMsg.m_s); it != m_p->m_tickPrices.end()) {
                it->second.m_a = eventMsg.m_a;
                it->second.m_b = eventMsg.m_b;
                it->second.m_u = eventMsg.m_u;
                it->second.m_T = eventMsg.m_T;
                it->second.m_E = eventMsg.m_E;
                it->second.m_e = eventMsg.m_e;

                /// Accumulate volume between read outs, otherwise the volume information would be lost!
                it->second.m_A += eventMsg.m_A;
                it->second.m_B += eventMsg.m_B;
            } else {
                m_p->m_tickPrices.insert({eventMsg.m_s, eventMsg});
            }
        } catch (nlohmann::json::exception &e) {
            m_p->m_logMessageCB(LogSeverity::Error, fmt::format("{}: {}", MAKE_FILELINE, e.what()));
        }
    });

    m_p->m_wsClient->run();
}

void WSStreamManager::subscribeCandlestickStream(const std::string &pair, const CandleInterval interval, bool) const {
    std::string channel("kline");
    channel.append(magic_enum::enum_name(interval));

    if (m_p->m_wsClient->findStream(WebSocketClient::composeStreamName(pair, channel))) {
        return;
    }

    if (m_p->m_logMessageCB) {
        const auto msgString = fmt::format("subscribing: {}", WebSocketClient::composeStreamName(pair, channel));
        m_p->m_logMessageCB(LogSeverity::Info, msgString);
    }

    m_p->m_wsClient->candlestick(pair, interval, [&](const nlohmann::json &msg) {
        std::lock_guard lk(m_p->m_candlestickLocker);
        EventCandlestick eventMsg;
        eventMsg.fromJson(msg);

        try {
            std::optional<EventCandlestick> previousCandle;
            /// Insert new candle
            {
                auto it = m_p->m_candlesticks.find(eventMsg.m_s);

                if (it == m_p->m_candlesticks.end()) {
                    m_p->m_candlesticks.insert({eventMsg.m_s, {}});
                }

                it = m_p->m_candlesticks.find(eventMsg.m_s);

                if (const auto itInterval = it->second.find(eventMsg.m_k.m_i);
                    itInterval != it->second.end() && itInterval->second.m_k.m_t != eventMsg.m_k.m_t) {
                    previousCandle = itInterval->second;
                }

                it->second.insert_or_assign(eventMsg.m_k.m_i, eventMsg);
            }

            /// Update historic candle
            {
                if (previousCandle) {
                    auto it = m_p->m_candlesticksHistoric.find(eventMsg.m_s);

                    if (it == m_p->m_candlesticksHistoric.end()) {
                        m_p->m_candlesticksHistoric.insert({eventMsg.m_s, {}});
                    }

                    it = m_p->m_candlesticksHistoric.find(eventMsg.m_s);
                    it->second.insert_or_assign(previousCandle->m_k.m_i,
                                                *previousCandle);
                }
            }
        } catch (nlohmann::json::exception &e) {
            m_p->m_logMessageCB(LogSeverity::Error, fmt::format("{}: {}", MAKE_FILELINE, e.what()));
        }
    });

    m_p->m_wsClient->run();
}

void WSStreamManager::setTimeout(const int seconds) const {
    m_p->m_timeout = seconds;
}

int WSStreamManager::timeout() const {
    return m_p->m_timeout;
}

void WSStreamManager::setLoggerCallback(const onLogMessage &onLogMessageCB) const {
    m_p->m_logMessageCB = onLogMessageCB;
    m_p->m_wsClient->setLoggerCallback(onLogMessageCB);
}

std::optional<EventTickPrice> WSStreamManager::readEventTickPrice(const std::string &pair,
                                                                  const bool consumeEvent) const {
    int numTries = 0;
    const int maxNumTries = static_cast<int>(m_p->m_timeout / 0.01);

    while (numTries <= maxNumTries) {
        if (m_p->m_timeout == 0) {
            /// No need to wait when destroying object
            break;
        }

        m_p->m_tickerLocker.lock();

        if (const auto it = m_p->m_tickPrices.find(pair); it != m_p->m_tickPrices.end()) {
            auto retVal = it->second;
            /// Zero volume counters - prepare for accumulating
            it->second.m_A = 0.0;
            it->second.m_B = 0.0;

            if (consumeEvent) {
                m_p->m_tickPrices.erase(it);
            }

            m_p->m_tickerLocker.unlock();
            return retVal;
        }
        m_p->m_tickerLocker.unlock();
        numTries++;
        std::this_thread::sleep_for(3ms);
    }

    return {};
}

std::optional<EventCandlestick>
WSStreamManager::readEventCandlestick(const std::string &pair, const CandleInterval interval,
                                      const bool previous) const {
    int numTries = 0;
    const int maxNumTries = static_cast<int>(m_p->m_timeout / 0.01);

    while (numTries <= maxNumTries) {
        if (m_p->m_timeout == 0) {
            /// No need to wait when destroying object
            break;
        }

        if (previous) {
            m_p->m_candlestickLocker.lock();

            if (const auto it = m_p->m_candlesticksHistoric.find(pair); it != m_p->m_candlesticksHistoric.end()) {
                if (const auto itCandle = it->second.find(interval); itCandle != it->second.end()) {
                    auto retVal = itCandle->second;
                    m_p->m_candlestickLocker.unlock();
                    return retVal;
                }
            }
            m_p->m_candlestickLocker.unlock();
        } else {
            m_p->m_candlestickLocker.lock();

            if (const auto it = m_p->m_candlesticks.find(pair); it != m_p->m_candlesticks.end()) {
                if (const auto itCandle = it->second.find(interval); itCandle != it->second.end()) {
                    auto retVal = itCandle->second;
                    m_p->m_candlestickLocker.unlock();
                    return retVal;
                }
            }
            m_p->m_candlestickLocker.unlock();
        }
        numTries++;
        std::this_thread::sleep_for(3ms);
    }

    return {};
}
}
