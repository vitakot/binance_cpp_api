/**
Binance HTTPS Session

Licensed under the MIT License <http://opensource.org/licenses/MIT>.
SPDX-License-Identifier: MIT
Copyright (c) 2022 Vitezslav Kot <vitezslav.kot@gmail.com>.
*/

#ifndef INCLUDE_VK_BINANCE_HTTP_SESSION_H
#define INCLUDE_VK_BINANCE_HTTP_SESSION_H

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <string>

namespace vk::binance {
namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;

class HTTPSession {
    struct P;
    std::unique_ptr<P> m_p{};

public:
    HTTPSession(const std::string &apiKey, const std::string &apiSecret, bool futures);

    ~HTTPSession();

    [[nodiscard]] http::response<http::string_body> get(const std::string &target, bool isPublic) const;

    [[nodiscard]] http::response<http::string_body> getV2(const std::string &target, bool isPublic) const;

    [[nodiscard]] http::response<http::string_body> getFutures(const std::string &target) const;

    [[nodiscard]] http::response<http::string_body> post(const std::string &target, const std::string &payload,
                                                         bool isPublic) const;

    [[nodiscard]] http::response<http::string_body> put(const std::string &target, const std::string &payload,
                                                        bool isPublic) const;

    [[nodiscard]] http::response<http::string_body> del(const std::string &target, bool isPublic) const;

    void setWeightLimit(std::int32_t weightLimit) const;

    [[nodiscard]] std::int32_t getUsedWeight() const;
};
}
#endif //INCLUDE_VK_BINANCE_HTTP_SESSION_H
