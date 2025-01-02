/**
Binance Futures WebSocket Session

Licensed under the MIT License <http://opensource.org/licenses/MIT>.
SPDX-License-Identifier: MIT
Copyright (c) 2022 Vitezslav Kot <vitezslav.kot@gmail.com>.
*/

#include "vk/binance/binance_futures_ws_session.h"
#include "vk/tools//log_utils.h"
#include "vk/tools//json_utils.h"
#include <fmt/format.h>
#include <nlohmann/json.hpp>
#include <boost/asio/buffers_iterator.hpp>
#include <boost/asio/strand.hpp>
#include <utility>

namespace vk::binance::futures {
static constexpr int PING_INTERVAL_IN_S = 10;

WebSocketSession::WebSocketSession(boost::asio::io_context &ioc, boost::asio::ssl::context &ctx,
                                   const onLogMessage &onLogMessageCB) : m_resolver(make_strand(ioc)),
                                                                         m_ws(make_strand(ioc), ctx),
                                                                         m_pingTimer(ioc, boost::asio::chrono::seconds(
                                                                                 PING_INTERVAL_IN_S)) {
    m_logMessageCB = onLogMessageCB;
}

WebSocketSession::~WebSocketSession() {
    m_pingTimer.cancel();
}

bool WebSocketSession::isApiError(const nlohmann::json &json) {
    return json.contains("code") && json.contains("msg");
}

std::pair<int, std::string> WebSocketSession::constructError(const nlohmann::json &json) {
    int ec;
    std::string msg;
    readValue<int>(json, "code", ec);
    readValue<std::string>(json, "msg", msg);
    return std::make_pair(ec, std::move(msg));
}

std::string WebSocketSession::target() const {
    return m_target;
}

void WebSocketSession::run(const std::string &host, const std::string &port, const std::string &target,
                           const onJSONMessage &onJsonMsg) {
    m_host = host;
    m_target = target;
    m_onJsonMsg = onJsonMsg;

    /// Look up the domain name
    m_resolver.async_resolve(host, port,
                             boost::beast::bind_front_handler(&WebSocketSession::onResolve, shared_from_this()));
}

void
WebSocketSession::onResolve(const boost::beast::error_code &ec,
                            const boost::asio::ip::tcp::resolver::results_type &results) {
    if (ec) {
        return m_logMessageCB(LogSeverity::Error, fmt::format("{}: {}", MAKE_FILELINE, ec.message()));
    }

    /// Set a timeout on the operation
    get_lowest_layer(m_ws).expires_after(std::chrono::seconds(30));

    /// Make the connection on the IP address we get from a lookup
    get_lowest_layer(m_ws).async_connect(results,
                                         boost::beast::bind_front_handler(&WebSocketSession::onConnect,
                                                                          shared_from_this()));
}

void WebSocketSession::onConnect(boost::beast::error_code ec,
                                 const boost::asio::ip::tcp::resolver::results_type::endpoint_type &ep) {
    if (ec) {
        return m_logMessageCB(LogSeverity::Error, fmt::format("{}: {}", MAKE_FILELINE, ec.message()));
    }

    /// Set a timeout on the operation
    get_lowest_layer(m_ws).expires_after(std::chrono::seconds(30));

    /// Set SNI Hostname (many hosts need this to handshake successfully)
    if (!SSL_set_tlsext_host_name(m_ws.next_layer().native_handle(), m_host.c_str())) {
        ec = boost::beast::error_code(static_cast<int>(ERR_get_error()), boost::asio::error::get_ssl_category());
        return m_logMessageCB(LogSeverity::Error, fmt::format("{}: {}", MAKE_FILELINE, ec.message()));
    }

    /// Update the host_ string. This will provide the value of the
    /// Host HTTP header during the WebSocket handshake.
    /// See https://tools.ietf.org/html/rfc7230#section-5.4
    m_host += ':' + std::to_string(ep.port());

    /// Perform the SSL handshake
    m_ws.next_layer().async_handshake(boost::asio::ssl::stream_base::client,
                                      boost::beast::bind_front_handler(&WebSocketSession::onSSLHandshake,
                                                                       shared_from_this()));
}

void WebSocketSession::onSSLHandshake(const boost::beast::error_code &ec) {
    if (ec) {
        return m_logMessageCB(LogSeverity::Error, fmt::format("{}: {}", MAKE_FILELINE, ec.message()));
    }

    m_ws.control_callback([this](boost::beast::websocket::frame_type kind, boost::beast::string_view payload) {
        boost::ignore_unused(kind, payload);

        if (kind == boost::beast::websocket::frame_type::pong) {
            m_lastPongTime = std::chrono::system_clock::now();
        }
    });

    /// Turn off the timeout on the tcp_stream, because
    /// the websocket stream has its own timeout system.
    get_lowest_layer(m_ws).expires_never();

    /// Set suggested timeout settings for the websocket
    m_ws.set_option(boost::beast::websocket::stream_base::timeout::suggested(boost::beast::role_type::client));

    /// Set a decorator to change the User-Agent of the handshake
    m_ws.set_option(boost::beast::websocket::stream_base::decorator([](boost::beast::websocket::request_type &req) {
        req.set(boost::beast::http::field::user_agent,
                std::string(BOOST_BEAST_VERSION_STRING) + " binance-client");
    }));

    /// Perform the websocket handshake
    m_ws.async_handshake(m_host, m_target,
                         boost::beast::bind_front_handler(&WebSocketSession::onHandshake, shared_from_this()));
}

void WebSocketSession::onHandshake(const boost::beast::error_code &ec) {
    if (ec) {
        return m_logMessageCB(LogSeverity::Error, fmt::format("{}: {}", MAKE_FILELINE, ec.message()));
    }

    /// Start reading the messages
    m_ws.async_read(m_buffer, boost::beast::bind_front_handler(&WebSocketSession::onRead, shared_from_this()));
    m_pingTimer.async_wait(boost::beast::bind_front_handler(&WebSocketSession::onPingTimer, shared_from_this()));
}

void WebSocketSession::onRead(const boost::beast::error_code &ec, std::size_t bytes_transferred) {
    boost::ignore_unused(bytes_transferred);

    if (ec) {
        m_pingTimer.cancel();
        return m_logMessageCB(LogSeverity::Error, fmt::format("{}: {}", MAKE_FILELINE, ec.message()));
    }

    try {
        const auto size = m_buffer.size();
        std::string strBuffer;
        strBuffer.reserve(size);

        for (const auto &it: m_buffer.data()) {
            strBuffer.append(static_cast<const char *>(it.data()), it.size());
        }

        m_buffer.consume(m_buffer.size());

        if (const nlohmann::json json = nlohmann::json::parse(strBuffer); json.is_object()) {
            if (isApiError(json)) {
                auto [fst, snd] = constructError(json);
                auto errorCode = fst;
                auto errorMsg = std::move(snd);
                m_logMessageCB(LogSeverity::Error, fmt::format("Binance API Error {}: {}", errorCode, errorMsg));
            } else {
                m_onJsonMsg(json);
            }
        }

        /// Start reading the messages
        m_ws.async_read(m_buffer, boost::beast::bind_front_handler(&WebSocketSession::onRead, shared_from_this()));
    } catch (nlohmann::json::exception &e) {
        m_logMessageCB(LogSeverity::Error, fmt::format("{}: {}", MAKE_FILELINE, e.what()));
        m_ws.async_close(boost::beast::websocket::close_code::normal,
                         boost::beast::bind_front_handler(&WebSocketSession::onClose, shared_from_this()));
    }
}

void WebSocketSession::ping() {
    if (const std::chrono::duration<double> elapsed = m_lastPingTime - m_lastPongTime;
        elapsed.count() > PING_INTERVAL_IN_S) {
        m_logMessageCB(LogSeverity::Warning, fmt::format("{}: {}", MAKE_FILELINE, "ping expired"));
    }

    if (m_ws.is_open()) {
        const boost::beast::websocket::ping_data pingWebSocketFrame;
        m_ws.async_ping(pingWebSocketFrame, [this](const boost::beast::error_code &ec) {
                            if (ec) {
                                m_logMessageCB(LogSeverity::Error, fmt::format("{}: {}", MAKE_FILELINE, ec.message()));
                            } else {
                                m_lastPingTime = std::chrono::system_clock::now();
                            }
                        }
        );
    }
}

void WebSocketSession::close() {
    m_ws.async_close(boost::beast::websocket::close_code::normal,
                     boost::beast::bind_front_handler(&WebSocketSession::onClose, shared_from_this()));
}

void WebSocketSession::onClose(const boost::beast::error_code &ec) {
    m_pingTimer.cancel();

    if (ec) {
        return m_logMessageCB(LogSeverity::Error, fmt::format("{}: {}", MAKE_FILELINE, ec.message()));
    }
}

void WebSocketSession::onPingTimer(const boost::beast::error_code &ec) {
    if (ec) {
        return m_logMessageCB(LogSeverity::Error, fmt::format("{}: {}", MAKE_FILELINE, ec.message()));
    }

    ping();
    m_pingTimer.expires_from_now(boost::asio::chrono::seconds(PING_INTERVAL_IN_S));
    m_pingTimer.async_wait(boost::beast::bind_front_handler(&WebSocketSession::onPingTimer, shared_from_this()));
}
}
