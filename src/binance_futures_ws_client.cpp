/**
Binance Futures WebSocket Client

Licensed under the MIT License <http://opensource.org/licenses/MIT>.
SPDX-License-Identifier: MIT
Copyright (c) 2022 Vitezslav Kot <vitezslav.kot@gmail.com>.
*/

#include "vk/binance/binance_futures_ws_client.h"
#include "vk/utils/json_utils.h"
#include <boost/beast/core.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <fmt/format.h>
#include <thread>

using namespace std::chrono_literals;

namespace vk::binance::futures {
#define STRINGIZE_I(x) #x
#define STRINGIZE(x) STRINGIZE_I(x)

#define MAKE_FILELINE \
    __FILE__ "(" STRINGIZE(__LINE__) ")"

static auto BINANCE_FUTURES_WS_HOST = "fstream.binance.com";
static auto BINANCE_FUTURES_WS_PORT = "443";

struct WebSocketClient::P {
    boost::asio::io_context m_ioContext;
    boost::asio::ssl::context m_ctx;
    std::string m_host = {BINANCE_FUTURES_WS_HOST};
    std::string m_port = {BINANCE_FUTURES_WS_PORT};
    std::vector<std::weak_ptr<WebSocketSession> > m_sessions;
    std::thread m_ioThread;
    std::atomic<bool> m_isRunning = false;
    onLogMessage m_logMessageCB;

    void removeDeadWebsockets() {
        for (auto it = m_sessions.begin(); it != m_sessions.end();) {
            if (!it->lock()) {
                it = m_sessions.erase(it);
            } else {
                ++it;
            }
        }
    }

    P() : m_ctx(boost::asio::ssl::context::sslv23_client) {
    }
};

WebSocketClient::WebSocketClient() : m_p(std::make_unique<P>()) {
}

WebSocketClient::~WebSocketClient() {
    m_p->m_ioContext.stop();

    if (m_p->m_ioThread.joinable()) {
        m_p->m_ioThread.join();
    }
}

std::string WebSocketClient::composeStreamName(const std::string &pair, const std::string &channel) {
    std::string res{"/ws/"};
    if (!pair.empty()) {
        res += pair;
        if (pair != "!") {
            boost::algorithm::to_lower(res);
        }

        res += '@';
    }

    res += channel;

    return res;
}

void WebSocketClient::run() const {
    if (m_p->m_isRunning) {
        return;
    }

    m_p->m_isRunning = true;

    if (m_p->m_ioThread.joinable()) {
        m_p->m_ioThread.join();
    }

    m_p->m_ioThread = std::thread([&] {
        for (;;) {
            try {
                m_p->m_isRunning = true;

                if (m_p->m_ioContext.stopped()) {
                    m_p->m_ioContext.restart();
                }
                m_p->m_ioContext.run();
                m_p->m_isRunning = false;
                break;
            } catch (std::exception &e) {
                if (m_p->m_logMessageCB) {
                    m_p->m_logMessageCB(LogSeverity::Error, fmt::format("{}: {}\n", MAKE_FILELINE, e.what()));
                }
            }
        }

        m_p->m_isRunning = false;
    });
}

void WebSocketClient::setLoggerCallback(const onLogMessage &onLogMessageCB) const {
    m_p->m_logMessageCB = onLogMessageCB;
}

bool WebSocketClient::findStream(const std::string &streamName) const {
    for (auto &m_session: m_p->m_sessions) {
        if (const auto session = m_session.lock()) {
            if (session->target() == streamName) {
                return true;
            }
        }
    }

    return false;
}

void WebSocketClient::bookTicker(const std::string &pair, const onJSONMessage &cb) const {
    m_p->removeDeadWebsockets();
    const std::string streamName = composeStreamName(pair, "bookTicker");
    const auto ws = std::make_shared<WebSocketSession>(m_p->m_ioContext, m_p->m_ctx, m_p->m_logMessageCB);
    std::weak_ptr wp{ws};
    m_p->m_sessions.emplace_back(std::move(wp));
    ws->run(BINANCE_FUTURES_WS_HOST, BINANCE_FUTURES_WS_PORT, streamName, cb);
}

void WebSocketClient::candlestick(const std::string &pair, const CandleInterval interval,
                                  const onJSONMessage &cb) const {
    m_p->removeDeadWebsockets();
    std::string channel("kline");
    channel.append(magic_enum::enum_name(interval));
    const std::string streamName = composeStreamName(pair, channel);
    const auto ws = std::make_shared<WebSocketSession>(m_p->m_ioContext, m_p->m_ctx, m_p->m_logMessageCB);
    std::weak_ptr wp{ws};
    m_p->m_sessions.emplace_back(std::move(wp));
    ws->run(BINANCE_FUTURES_WS_HOST, BINANCE_FUTURES_WS_PORT, streamName, cb);
}

void WebSocketClient::partialBookDepthStream(const std::string &pair, int depth, const onJSONMessage &cb) const {
    if (depth != 5 && depth != 10 && depth != 20) {
        throw std::invalid_argument(fmt::format("invalid depth parameter, must be 5, 10 or 20, is {}", depth));
    }

    m_p->removeDeadWebsockets();
    std::string channel("depth");
    channel.append(std::to_string(depth));
    const std::string streamName = composeStreamName(pair, channel);
    const auto ws = std::make_shared<WebSocketSession>(m_p->m_ioContext, m_p->m_ctx, m_p->m_logMessageCB);
    std::weak_ptr wp{ws};
    m_p->m_sessions.emplace_back(std::move(wp));
    ws->run(BINANCE_FUTURES_WS_HOST, BINANCE_FUTURES_WS_PORT, streamName, cb);
}
}
