/**
Binance Futures WebSocket Session

Licensed under the MIT License <http://opensource.org/licenses/MIT>.
SPDX-License-Identifier: MIT
Copyright (c) 2022 Vitezslav Kot <vitezslav.kot@gmail.com>.
*/

#include "vk/binance/binance_futures_ws_session.h"
#include "vk/utils/log_utils.h"
#include "vk/utils/json_utils.h"
#include <nlohmann/json.hpp>
#include <boost/asio/buffers_iterator.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>

namespace vk::binance::futures {
static constexpr int PING_INTERVAL_IN_S = 10;

struct WebSocketSession::P {
    boost::asio::ip::tcp::resolver resolver;
    boost::beast::websocket::stream<boost::beast::ssl_stream<boost::beast::tcp_stream>> ws;
    boost::beast::multi_buffer buffer;
    std::string host;
    std::string target;
    std::string streamName;
    onLogMessage logMessageCB;
    std::function<void(const nlohmann::json &msg)> onJsonMsg;
    boost::asio::steady_timer pingTimer;
    std::chrono::time_point<std::chrono::system_clock> lastPingTime{};
    std::chrono::time_point<std::chrono::system_clock> lastPongTime{};

    P(boost::asio::io_context &ioc, boost::asio::ssl::context &ctx, const onLogMessage &onLogMessageCB) :
        resolver(make_strand(ioc)), ws(make_strand(ioc), ctx), logMessageCB(onLogMessageCB), pingTimer(ioc, boost::asio::chrono::seconds(PING_INTERVAL_IN_S)) {}

    static std::pair<int, std::string> constructError(const nlohmann::json &json) {
        int ec;
        std::string msg;
        readValue<int>(json, "code", ec);
        readValue<std::string>(json, "msg", msg);
        return std::make_pair(ec, std::move(msg));
    }

    static bool isApiError(const nlohmann::json &json) { return json.contains("code") && json.contains("msg"); }

    void onResolve(const std::shared_ptr<WebSocketSession> &self, const boost::beast::error_code &ec, const boost::asio::ip::tcp::resolver::results_type &results) {
        if (ec) {
            return logMessageCB(LogSeverity::Error, fmt::format("{}: {}", MAKE_FILELINE, ec.message()));
        }

        get_lowest_layer(ws).expires_after(std::chrono::seconds(30));

        get_lowest_layer(ws).async_connect(
                results, [this, self](const boost::beast::error_code &e, const boost::asio::ip::tcp::resolver::results_type::endpoint_type &ep) { onConnect(self, e, ep); });
    }

    void onConnect(const std::shared_ptr<WebSocketSession> &self, boost::beast::error_code ec, const boost::asio::ip::tcp::resolver::results_type::endpoint_type &ep) {
        if (ec) {
            return logMessageCB(LogSeverity::Error, fmt::format("{}: {}", MAKE_FILELINE, ec.message()));
        }

        get_lowest_layer(ws).expires_after(std::chrono::seconds(30));

        if (!SSL_set_tlsext_host_name(ws.next_layer().native_handle(), host.c_str())) {
            ec = boost::beast::error_code(static_cast<int>(ERR_get_error()), boost::asio::error::get_ssl_category());
            return logMessageCB(LogSeverity::Error, fmt::format("{}: {}", MAKE_FILELINE, ec.message()));
        }

        host += ':' + std::to_string(ep.port());

        ws.next_layer().async_handshake(boost::asio::ssl::stream_base::client, [this, self](const boost::beast::error_code &e) { onSSLHandshake(self, e); });
    }

    void onSSLHandshake(const std::shared_ptr<WebSocketSession> &self, const boost::beast::error_code &ec) {
        if (ec) {
            return logMessageCB(LogSeverity::Error, fmt::format("{}: {}", MAKE_FILELINE, ec.message()));
        }

        ws.control_callback([this](boost::beast::websocket::frame_type kind, boost::beast::string_view payload) {
            boost::ignore_unused(kind, payload);

            if (kind == boost::beast::websocket::frame_type::pong) {
                lastPongTime = std::chrono::system_clock::now();
            }
        });

        get_lowest_layer(ws).expires_never();

        ws.set_option(boost::beast::websocket::stream_base::timeout::suggested(boost::beast::role_type::client));

        ws.set_option(boost::beast::websocket::stream_base::decorator(
                [](boost::beast::websocket::request_type &req) { req.set(boost::beast::http::field::user_agent, std::string(BOOST_BEAST_VERSION_STRING) + " binance-client"); }));

        ws.async_handshake(host, target, [this, self](const boost::beast::error_code &e) { onHandshake(self, e); });
    }

    void onHandshake(const std::shared_ptr<WebSocketSession> &self, const boost::beast::error_code &ec) {
        if (ec) {
            return logMessageCB(LogSeverity::Error, fmt::format("{}: {}", MAKE_FILELINE, ec.message()));
        }

        ws.async_read(buffer, [this, self](const boost::beast::error_code &e, const std::size_t bytesTransferred) { onRead(self, e, bytesTransferred); });
        pingTimer.async_wait([this, self](const boost::beast::error_code &e) { onPingTimer(self, e); });
    }

    void onRead(const std::shared_ptr<WebSocketSession> &self, const boost::beast::error_code &ec, std::size_t bytesTransferred) {
        boost::ignore_unused(bytesTransferred);

        if (ec) {
            pingTimer.cancel();
            return logMessageCB(LogSeverity::Error, fmt::format("{}: {}", MAKE_FILELINE, ec.message()));
        }

        try {
            const auto size = buffer.size();
            std::string strBuffer;
            strBuffer.reserve(size);

            for (const auto &it: buffer.data()) {
                strBuffer.append(static_cast<const char *>(it.data()), it.size());
            }

            buffer.consume(buffer.size());

            if (const nlohmann::json json = nlohmann::json::parse(strBuffer); json.is_object()) {
                if (isApiError(json)) {
                    auto [fst, snd] = constructError(json);
                    auto errorCode = fst;
                    auto errorMsg = std::move(snd);
                    logMessageCB(LogSeverity::Error, fmt::format("Binance API Error {}: {}", errorCode, errorMsg));
                } else {
                    onJsonMsg(json);
                }
            }

            ws.async_read(buffer, [this, self](const boost::beast::error_code &e, const std::size_t transferred) { onRead(self, e, transferred); });
        } catch (nlohmann::json::exception &exc) {
            logMessageCB(LogSeverity::Error, fmt::format("{}: {}", MAKE_FILELINE, exc.what()));
            ws.async_close(boost::beast::websocket::close_code::normal, [this](const boost::beast::error_code &e) { onClose(e); });
        }
    }

    void ping() {
        if (const std::chrono::duration<double> elapsed = lastPingTime - lastPongTime; elapsed.count() > PING_INTERVAL_IN_S) {
            logMessageCB(LogSeverity::Warning, fmt::format("{}: {}", MAKE_FILELINE, "ping expired"));
        }

        if (ws.is_open()) {
            const boost::beast::websocket::ping_data pingWebSocketFrame;
            ws.async_ping(pingWebSocketFrame, [this](const boost::beast::error_code &ec) {
                if (ec) {
                    logMessageCB(LogSeverity::Error, fmt::format("{}: {}", MAKE_FILELINE, ec.message()));
                } else {
                    lastPingTime = std::chrono::system_clock::now();
                }
            });
        }
    }

    void closeWs() {
        ws.async_close(boost::beast::websocket::close_code::normal, [this](const boost::beast::error_code &ec) { onClose(ec); });
    }

    void onClose(const boost::beast::error_code &ec) {
        pingTimer.cancel();

        if (ec) {
            return logMessageCB(LogSeverity::Error, fmt::format("{}: {}", MAKE_FILELINE, ec.message()));
        }
    }

    void onPingTimer(const std::shared_ptr<WebSocketSession> &self, const boost::beast::error_code &ec) {
        if (ec) {
            return logMessageCB(LogSeverity::Error, fmt::format("{}: {}", MAKE_FILELINE, ec.message()));
        }

        ping();
        pingTimer.expires_after(boost::asio::chrono::seconds(PING_INTERVAL_IN_S));
        pingTimer.async_wait([this, self](const boost::beast::error_code &e) { onPingTimer(self, e); });
    }
};

WebSocketSession::WebSocketSession(boost::asio::io_context &ioc, boost::asio::ssl::context &ctx, const onLogMessage &onLogMessageCB) :
    m_p(std::make_unique<P>(ioc, ctx, onLogMessageCB)) {}

WebSocketSession::~WebSocketSession() { m_p->pingTimer.cancel(); }

std::string WebSocketSession::target() const { return m_p->target; }

void WebSocketSession::run(const std::string &host, const std::string &port, const std::string &target, const onJSONMessage &onJsonMsg) {
    m_p->host = host;
    m_p->target = target;
    m_p->onJsonMsg = onJsonMsg;

    auto self = shared_from_this();
    m_p->resolver.async_resolve(
            host, port, [this, self](const boost::beast::error_code &ec, const boost::asio::ip::tcp::resolver::results_type &results) { m_p->onResolve(self, ec, results); });
}

void WebSocketSession::close() const { m_p->closeWs(); }
} // namespace vk::binance::futures
