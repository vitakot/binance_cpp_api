/**
Binance Futures WebSocket Stream manager

Licensed under the MIT License <http://opensource.org/licenses/MIT>.
SPDX-License-Identifier: MIT
Copyright (c) 2022 Vitezslav Kot <vitezslav.kot@gmail.com>.
*/

#ifndef INCLUDE_VK_BINANCE_FUTURES_WS_STREAM_MANAGER_H
#define INCLUDE_VK_BINANCE_FUTURES_WS_STREAM_MANAGER_H

#include <vk/tools/log_utils.h>
#include "binance_event_models.h"
#include "binance_models.h"
#include <optional>

namespace vk::binance::futures {
class RESTClient;

class WSStreamManager {
    struct P;
    std::unique_ptr<P> m_p{};

public:
    explicit WSStreamManager(const std::weak_ptr<RESTClient> &restClient);

    ~WSStreamManager();

    /**
     * Check if the Book Ticker Stream is subscribed for a selected pair, if not then subscribe it. When force parameter
     * is true then re-subscribe if already subscribed
     * @param pair e.g BTCUSDT
     * @param force If true then re-subscribe if already subscribed
     */
    void subscribeBookTickerStream(const std::string &pair, bool force = false) const;

    /**
     * Check if the Candlestick Stream is subscribed for a selected pair, if not then subscribe it. When force parameter
     * is true then re-subscribe if already subscribed
     * @param pair e.g BTCUSDT
     * @param interval e.g CandleInterval::_1m
     * @param force If true then re-subscribe if already subscribed
     */
    void subscribeCandlestickStream(const std::string &pair, CandleInterval interval, bool force = false) const;

    /**
     * Set time of all reading operations
     * @param seconds
     */
    void setTimeout(int seconds) const;

    /**
     * Get time of all reading operations
     * @return
     */
    [[nodiscard]] int timeout() const;

    /**
     * Set logger callback, if no set then all errors are writen to the stderr stream only
     * @param onLogMessageCB
     */
    void setLoggerCallback(const onLogMessage &onLogMessageCB) const;

    /**
     * Try to read EventTickPrice structure. It will block at most Timeout time.
     * @param pair e.g BTCUSDT
     * @param consumeEvent If true then the event is taken from the underlying list which means that subsequent
     * call of this method returns bad option if no data arrive before the call.
     * @return EventTickPrice structure if successful
     */
    [[nodiscard]] std::optional<EventTickPrice> readEventTickPrice(const std::string &pair,
                                                                   bool consumeEvent = false) const;

    /**
     * Try to read EventCandlestick structure. It will block at most Timeout time.
     * @param pair e.g BTCUSDT
     * @param interval e.g CandleInterval::_1m
     * @param previous if true then return interval-1 candle
     * @return EventCandlestick structure if successful
     */
    [[nodiscard]] std::optional<EventCandlestick>
    readEventCandlestick(const std::string &pair, CandleInterval interval, bool previous = false) const;
};
}

#endif //INCLUDE_VK_BINANCE_FUTURES_WS_STREAM_MANAGER_H
