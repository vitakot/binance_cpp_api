/**
Binance HTTPS Session

Licensed under the MIT License <http://opensource.org/licenses/MIT>.
SPDX-License-Identifier: MIT
Copyright (c) 2022 Vitezslav Kot <vitezslav.kot@gmail.com>.
*/

#include "vk/binance/binance_http_session.h"
#include "vk/utils/utils.h"
#include <boost/asio/ssl.hpp>
#include <boost/beast/version.hpp>
#include <spdlog/spdlog.h>
#include <openssl/hmac.h>

namespace vk::binance {
namespace ssl = boost::asio::ssl;
using tcp = net::ip::tcp;

auto API_SPOT_URI = "api.binance.com";
auto API_FUTURES_URI = "fapi.binance.com";

auto PRIVATE_API_SPOT = "/api/v3/";
auto PUBLIC_API_SPOT = "/api/v1/";

auto PRIVATE_API_FUTURES = "/fapi/v1/";
auto PUBLIC_API_FUTURES = "/fapi/v1/";

auto PRIVATE_API_FUTURES_V2 = "/fapi/v2/";
auto PUBLIC_API_FUTURES_V2 = "/fapi/v2/";

struct HTTPSession::P {
    net::io_context m_ioc;
    std::string m_apiKey;
    std::string m_apiSecret;
    std::string m_uri;
    std::string m_publicApi;
    std::string m_privateApi;
    std::string m_publicApiV2;
    std::string m_privateApiV2;
    std::atomic<std::int32_t> m_usedWeight = 0;
    std::atomic<std::tm> m_lastResponseTime{};
    std::int32_t m_weightLimit{};
    const EVP_MD *m_evp_md;

    P() : m_evp_md(EVP_sha256()) {
    }

    http::response<http::string_body> request(http::request<http::string_body> req);

    void addTimestampToTargetPath(std::string &target) const;
};

HTTPSession::HTTPSession(const std::string &apiKey, const std::string &apiSecret, const bool futures) : m_p(
    std::make_unique<P>()) {
    if (futures) {
        m_p->m_uri = API_FUTURES_URI;
        m_p->m_publicApi = PUBLIC_API_FUTURES;
        m_p->m_privateApi = PRIVATE_API_FUTURES;
        m_p->m_publicApiV2 = PUBLIC_API_FUTURES_V2;
        m_p->m_privateApiV2 = PRIVATE_API_FUTURES_V2;
    } else {
        m_p->m_uri = API_SPOT_URI;
        m_p->m_publicApi = PUBLIC_API_SPOT;
        m_p->m_privateApi = PRIVATE_API_SPOT;
        m_p->m_publicApiV2 = PUBLIC_API_SPOT;
        m_p->m_privateApiV2 = PRIVATE_API_SPOT;
    }

    m_p->m_apiKey = apiKey;
    m_p->m_apiSecret = apiSecret;

    /// 2400 is the default value according to https://binance-docs.github.io/apidocs/futures/en/#limits
    m_p->m_weightLimit = 2400 * 0.85;
    spdlog::info(fmt::format("API Weight limit: {}", m_p->m_weightLimit));
}

HTTPSession::~HTTPSession() = default;

http::response<http::string_body> HTTPSession::get(const std::string &target, const bool isPublic) const {
    std::string finalTarget = target;

    if (!isPublic) {
        m_p->addTimestampToTargetPath(finalTarget);
    }

    std::string endpoint;

    if (isPublic) {
        endpoint = m_p->m_publicApi + finalTarget;
    } else {
        endpoint = m_p->m_privateApi + finalTarget;
    }

    const http::request<http::string_body> req{http::verb::get, endpoint, 11};
    return m_p->request(req);
}

http::response<http::string_body> HTTPSession::getV2(const std::string &target, const bool isPublic) const {
    std::string finalTarget = target;

    if (!isPublic) {
        m_p->addTimestampToTargetPath(finalTarget);
    }

    std::string endpoint;

    if (isPublic) {
        endpoint = m_p->m_publicApiV2 + finalTarget;
    } else {
        endpoint = m_p->m_privateApiV2 + finalTarget;
    }

    const http::request<http::string_body> req{http::verb::get, endpoint, 11};
    return m_p->request(req);
}

http::response<http::string_body> HTTPSession::getFutures(const std::string &target) const {
    const http::request<http::string_body> req{http::verb::get, target, 11};
    return m_p->request(req);
}

http::response<http::string_body>
HTTPSession::post(const std::string &target, const std::string &payload, const bool isPublic) const {
    std::string finalTarget = target;

    if (!isPublic) {
        m_p->addTimestampToTargetPath(finalTarget);
    }

    std::string endpoint;

    if (isPublic) {
        endpoint = m_p->m_publicApi + finalTarget;
    } else {
        endpoint = m_p->m_privateApi + finalTarget;
    }

    http::request<http::string_body> req{http::verb::post, endpoint, 11};
    req.body() = payload;
    req.prepare_payload();
    return m_p->request(req);
}

http::response<http::string_body>
HTTPSession::put(const std::string &target, const std::string &payload, const bool isPublic) const {
    std::string finalTarget = target;

    if (!isPublic) {
        m_p->addTimestampToTargetPath(finalTarget);
    }

    std::string endpoint;

    if (isPublic) {
        endpoint = m_p->m_publicApi + finalTarget;
    } else {
        endpoint = m_p->m_privateApi + finalTarget;
    }

    http::request<http::string_body> req{http::verb::put, endpoint, 11};
    req.body() = payload;
    req.prepare_payload();
    return m_p->request(req);
}

http::response<http::string_body> HTTPSession::del(const std::string &target, const bool isPublic) const {
    std::string finalTarget = target;

    if (!isPublic) {
        m_p->addTimestampToTargetPath(finalTarget);
    }

    std::string endpoint;

    if (isPublic) {
        endpoint = m_p->m_publicApi + finalTarget;
    } else {
        endpoint = m_p->m_privateApi + finalTarget;
    }

    const http::request<http::string_body> req{http::verb::delete_, endpoint, 11};
    return m_p->request(req);
}

http::response<http::string_body> HTTPSession::P::request(
    http::request<http::string_body> req) {
    req.set(http::field::host, m_uri);
    req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

    ssl::context ctx{ssl::context::sslv23_client};
    ctx.set_default_verify_paths();

    tcp::resolver resolver{m_ioc};
    ssl::stream<tcp::socket> stream{m_ioc, ctx};

    // Set SNI Hostname (many hosts need this to handshake successfully)
    if (!SSL_set_tlsext_host_name(stream.native_handle(), m_uri.c_str())) {
        boost::system::error_code ec{
            static_cast<int>(ERR_get_error()),
            net::error::get_ssl_category()
        };
        throw boost::system::system_error{ec};
    }

    auto const results = resolver.resolve(m_uri, "443");
    net::connect(stream.next_layer(), results.begin(), results.end());
    stream.handshake(ssl::stream_base::client);

    req.set("X-MBX-APIKEY", m_apiKey);

    if (req.method() == http::verb::post) {
        req.set(http::field::content_type, "application/json");
    }

    http::write(stream, req);
    beast::flat_buffer buffer;
    http::response<http::string_body> response;
    http::read(stream, buffer, response);

    for (auto &h: response.base()) {
        if (h.name_string() == "X-MBX-USED-WEIGHT-1M") {
            m_usedWeight = std::stoi(std::string(h.value()));
        } else if (h.name_string() == "Date") {
            const auto dateString = std::string(h.value());
            std::string timeFormat = "%a, %d %b %Y %H:%M:%S";
            m_lastResponseTime = getTimeFromString(dateString, timeFormat);
        }
    }

    if (m_usedWeight >= m_weightLimit) {
        auto secToWeighReset = 60 - m_lastResponseTime.load().tm_sec;
        spdlog::warn(fmt::format("Weigh limit reached, waiting for reset {} seconds", secToWeighReset));
        std::this_thread::sleep_for(std::chrono::seconds(secToWeighReset));
    }

    boost::system::error_code ec;
    stream.shutdown(ec);
    if (ec == boost::asio::error::eof) {
        // Rationale:
        // http://stackoverflow.com/questions/25587403/boost-asio-ssl-async-shutdown-always-finishes-with-an-error
        ec.assign(0, ec.category());
    }

    return response;
}

void HTTPSession::P::addTimestampToTargetPath(std::string &target) const {
    std::string parameters = target.substr(target.find('?') + 1);
    const std::string path = target.substr(0, target.find('?') + 1);

    parameters.append("&recvWindow=");
    parameters.append(std::to_string(60000));

    parameters.append("&timestamp=");
    parameters.append(std::to_string(getMsTimestamp(currentTime()).count()));

    unsigned char digest[SHA256_DIGEST_LENGTH];
    unsigned int digestLength = SHA256_DIGEST_LENGTH;

    HMAC(m_evp_md, m_apiSecret.data(), m_apiSecret.size(),
         reinterpret_cast<const unsigned char *>(parameters.data()),
         parameters.length(), digest, &digestLength);

    const std::string signature = stringToHex(digest, sizeof(digest));

    parameters.append("&signature=");
    parameters.append(signature);

    target = path + parameters;
}

void HTTPSession::setWeightLimit(const std::int32_t weightLimit) const {
    m_p->m_weightLimit = static_cast<std::int32_t>(weightLimit * 0.95);
}

std::int32_t HTTPSession::getUsedWeight() const {
    return m_p->m_usedWeight;
}
}
