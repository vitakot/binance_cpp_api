/**
Binance Futures WebSocket Session

Licensed under the MIT License <http://opensource.org/licenses/MIT>.
SPDX-License-Identifier: MIT
Copyright (c) 2022 Vitezslav Kot <vitezslav.kot@gmail.com>.
*/

#ifndef INCLUDE_VK_BINANCE_WS_SESSION_H
#define INCLUDE_VK_BINANCE_WS_SESSION_H

#include "vk/utils/log_utils.h"
#include <boost/asio/io_context.hpp>
#include <boost/asio/ssl/context.hpp>
#include <memory>
#include <nlohmann/json_fwd.hpp>

namespace vk::binance::futures {
using onJSONMessage = std::function<void(const nlohmann::json &msg)>;

class WebSocketSession final : public std::enable_shared_from_this<WebSocketSession> {
    struct P;
    std::unique_ptr<P> m_p;

public:
    explicit WebSocketSession(boost::asio::io_context &ioc, boost::asio::ssl::context &ctx, const onLogMessage &onLogMessageCB);

    ~WebSocketSession();

    void run(const std::string &host, const std::string &port, const std::string &target, const onJSONMessage &onJsonMsg);

    void close() const;

    [[nodiscard]] std::string target() const;
};
} // namespace vk::binance::futures
#endif // INCLUDE_VK_BINANCE_WS_SESSION_H
