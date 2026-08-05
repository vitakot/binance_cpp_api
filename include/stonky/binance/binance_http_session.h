/**
Binance HTTPS Session

Licensed under the MIT License <http://opensource.org/licenses/MIT>.
SPDX-License-Identifier: MIT
Copyright (c) 2022 Vitezslav Kot <vitezslav.kot@stonky.cz>, Stonky s.r.o.
*/

#ifndef INCLUDE_STONKY_BINANCE_HTTP_SESSION_H
#define INCLUDE_STONKY_BINANCE_HTTP_SESSION_H

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <stdexcept>
#include <string>

namespace stonky::binance {
namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;

/**
 * Thrown when a request fails on the transport level - name resolution, TCP, TLS or a timeout. In contrast to an API
 * error this means the outcome is UNKNOWN: an order may or may not have reached the exchange, so the caller must not
 * treat it as a rejection.
 */
class TransportError final : public std::runtime_error {
public:
    explicit TransportError(const std::string &message) : std::runtime_error(message) {
    }
};

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
#endif //INCLUDE_STONKY_BINANCE_HTTP_SESSION_H
