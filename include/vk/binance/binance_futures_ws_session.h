/**
Binance Futures WebSocket Session

Licensed under the MIT License <http://opensource.org/licenses/MIT>.
SPDX-License-Identifier: MIT
Copyright (c) 2022 Vitezslav Kot <vitezslav.kot@gmail.com>.
*/

#ifndef INCLUDE_VK_BINANCE_WS_SESSION_H
#define INCLUDE_VK_BINANCE_WS_SESSION_H

#include "vk/utils/log_utils.h"
#include <memory>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>
#include <nlohmann/json_fwd.hpp>

namespace vk::binance::futures {
using onJSONMessage = std::function<void(const nlohmann::json &msg)>;

class WebSocketSession final : public std::enable_shared_from_this<WebSocketSession> {
    boost::asio::ip::tcp::resolver m_resolver;
    boost::beast::websocket::stream<boost::beast::ssl_stream<boost::beast::tcp_stream> > m_ws;
    boost::beast::multi_buffer m_buffer;
    std::string m_host;
    std::string m_target;
    std::string m_streamName;
    onLogMessage m_logMessageCB;
    std::function<void(const nlohmann::json &msg)> m_onJsonMsg;
    boost::asio::steady_timer m_pingTimer;
    std::chrono::time_point<std::chrono::system_clock> m_lastPingTime{};
    std::chrono::time_point<std::chrono::system_clock> m_lastPongTime{};

    static std::pair<int, std::string> constructError(const nlohmann::json &json);

    static bool isApiError(const nlohmann::json &json);

    void onResolve(const boost::beast::error_code &ec, const boost::asio::ip::tcp::resolver::results_type &results);

    void onConnect(boost::beast::error_code ec, const boost::asio::ip::tcp::resolver::results_type::endpoint_type &ep);

    void onSSLHandshake(const boost::beast::error_code &ec);

    void onHandshake(const boost::beast::error_code &ec);

    void onRead(const boost::beast::error_code &ec, std::size_t bytes_transferred);

    void onClose(const boost::beast::error_code &ec);

    void ping();

    void onPingTimer(const boost::beast::error_code &ec);

public:
    explicit WebSocketSession(boost::asio::io_context &ioc, boost::asio::ssl::context &ctx,
                              const onLogMessage &onLogMessageCB);

    ~WebSocketSession();

    void
    run(const std::string &host, const std::string &port, const std::string &target, const onJSONMessage &onJsonMsg);

    void close();

    std::string target() const;
};
}
#endif //INCLUDE_VK_BINANCE_WS_SESSION_H
