/**
Binance Futures WebSocket Client

Licensed under the MIT License <http://opensource.org/licenses/MIT>.
SPDX-License-Identifier: MIT
Copyright (c) 2022 Vitezslav Kot <vitezslav.kot@gmail.com>.
*/

#ifndef BINANCE_FUTURES_WS_CLIENT_H
#define BINANCE_FUTURES_WS_CLIENT_H

#include <vk/utils/log_utils.h>
#include "binance_models.h"
#include "binance_futures_ws_session.h"
#include <string>

namespace vk::binance::futures {
class WebSocketClient {
    struct P;
    std::unique_ptr<P> m_p{};

public:
    WebSocketClient(const WebSocketClient &) = delete;

    WebSocketClient &operator=(const WebSocketClient &) = delete;

    WebSocketClient(WebSocketClient &&) noexcept = default;

    WebSocketClient &operator=(WebSocketClient &&) noexcept = default;

    WebSocketClient();

    ~WebSocketClient();

    /**
     * Compose full stream name from pair symbol and channel name
     * @param pair e.g. "BTCUSDT"
     * @param channel e.g. bookTicker
     * @return full stream name
     */
    static std::string composeStreamName(const std::string &pair, const std::string &channel);

    /**
     * Run the WebSocket IO Context asynchronously and returns immediately without blocking the thread execution
     */
    void run() const;

    /**
     * Set logger callback, if no set then all errors are writen to the stderr stream only
     * @param onLogMessageCB
     */
    void setLoggerCallback(const onLogMessage &onLogMessageCB) const;

    [[nodiscard]] bool findStream(const std::string &streamName) const;

    /**
     * Subscribe WebSocket to the bookTicker data stream
     * @param pair currency pair e.g. BTCUSDT
     * @param cb handle to process the incoming data
     */
    void bookTicker(const std::string &pair, const onJSONMessage &cb) const;

    /**
    * Subscribe WebSocket to the kline/candlestick data stream
    * @param pair currency pair e.g. BTCUSDT
    * @param interval
    * @param cb handle to process the incoming data
    */
    void candlestick(const std::string &pair, CandleInterval interval, const onJSONMessage &cb) const;

    /**
     * Subscribe to Partial Book Depth Stream
     * @param pair currency pair e.g. BTCUSDT
     * @param depth stream depth 5, 10 or 20
     * @param cb handle to process the incoming data
     */
    void partialBookDepthStream(const std::string &pair, int depth, const onJSONMessage &cb) const;
};
}

#endif //BINANCE_FUTURES_WS_CLIENT_H
