/**
Binance HTTPS Session

Licensed under the MIT License <http://opensource.org/licenses/MIT>.
SPDX-License-Identifier: MIT
Copyright (c) 2022 Vitezslav Kot <vitezslav.kot@stonky.cz>, Stonky s.r.o.
*/

#include "stonky/binance/binance_http_session.h"
#include "stonky/binance/tls_verify.h"
#include "stonky/utils/utils.h"
#include "stonky/utils/json_utils.h"
#include <boost/asio/ssl.hpp>
#include <boost/beast/version.hpp>
#include <spdlog/spdlog.h>
#include <openssl/hmac.h>

namespace stonky::binance {
namespace ssl = boost::asio::ssl;
using tcp = net::ip::tcp;

auto API_SPOT_URI = "api.binance.com";
auto API_FUTURES_URI = "fapi.binance.com";

auto PRIVATE_API_SPOT = "/api/v3/";
auto PUBLIC_API_SPOT = "/api/v3/";

auto PRIVATE_API_FUTURES = "/fapi/v1/";
auto PUBLIC_API_FUTURES = "/fapi/v1/";

auto PRIVATE_API_FUTURES_V2 = "/fapi/v2/";
auto PUBLIC_API_FUTURES_V2 = "/fapi/v2/";

/// How often the local clock is re-synchronized against the exchange clock
static constexpr std::int64_t TIME_SYNC_INTERVAL_MS = 30 * 60 * 1000;

/// How long a delayed request may still be executed by the exchange. Binance defaults to 5000 ms and allows at most
/// 60000; the maximum would let a badly delayed order still be executed, which the time synchronization makes
/// unnecessary.
static constexpr int RECV_WINDOW_MS = 5000;

/**
 * Bound the blocking socket operations. Beast's synchronous calls carry no timeout of their own, so a black holed
 * connection blocks the calling thread - in the Zorro plugin that is the thread driving the whole strategy.
 *
 * NOTE: this is effective on Windows, where a timed out recv/send reports WSAETIMEDOUT and Asio surfaces it as an
 * error. On POSIX the same condition arrives as EAGAIN, which Asio cannot tell from a non-blocking would-block and
 * therefore polls and retries - there the call still blocks. Bounding POSIX as well needs the synchronous request
 * path rewritten to async operations driven by io_context::run_for.
 */
void applySocketTimeout(net::ip::tcp::socket &socket, const int timeoutMs) {
    if (timeoutMs <= 0) {
        return;
    }

#ifdef _WIN32
    const DWORD value = static_cast<DWORD>(timeoutMs);
    const auto data = reinterpret_cast<const char *>(&value);
    const int size = sizeof(value);
#else
    timeval value{};
    value.tv_sec = timeoutMs / 1000;
    value.tv_usec = timeoutMs % 1000 * 1000;
    const auto data = reinterpret_cast<const void *>(&value);
    const socklen_t size = sizeof(value);
#endif

    ::setsockopt(socket.native_handle(), SOL_SOCKET, SO_RCVTIMEO, data, size);
    ::setsockopt(socket.native_handle(), SOL_SOCKET, SO_SNDTIMEO, data, size);
}

struct HTTPSession::P {
    net::io_context ioc;
    std::string apiKey;
    std::string apiSecret;
    std::string uri;
    std::string publicApi;
    std::string privateApi;
    std::string publicApiV2;
    std::string privateApiV2;
    std::atomic<std::int32_t> usedWeight = 0;
    std::atomic<std::tm> lastResponseTime{};
    std::int32_t weightLimit{};
    const EVP_MD *evpMd;

    /// Difference between the exchange clock and the local clock. Signed requests are rejected with -1021 once the
    /// local clock drifts more than recvWindow away from the server, so the timestamps are corrected by this offset.
    mutable std::atomic<std::int64_t> timeOffsetMs{0};
    mutable std::atomic<std::int64_t> lastTimeSyncMs{0};
    std::atomic<std::int64_t> lastSuccessMs{0};
    std::atomic<int> requestTimeoutMs{DEFAULT_REQUEST_TIMEOUT_MS};
    const HTTPSession *parent{nullptr};

    P() : evpMd(EVP_sha256()) {
    }

    http::response<http::string_body> request(http::request<http::string_body> req);

    void addTimestampToTargetPath(std::string &target) const;

    void ensureTimeSync() const;
};

HTTPSession::HTTPSession(const std::string &apiKey, const std::string &apiSecret, const bool futures) : m_p(
    std::make_unique<P>()) {
    m_p->parent = this;

    if (futures) {
        m_p->uri = API_FUTURES_URI;
        m_p->publicApi = PUBLIC_API_FUTURES;
        m_p->privateApi = PRIVATE_API_FUTURES;
        m_p->publicApiV2 = PUBLIC_API_FUTURES_V2;
        m_p->privateApiV2 = PRIVATE_API_FUTURES_V2;
    } else {
        m_p->uri = API_SPOT_URI;
        m_p->publicApi = PUBLIC_API_SPOT;
        m_p->privateApi = PRIVATE_API_SPOT;
        m_p->publicApiV2 = PUBLIC_API_SPOT;
        m_p->privateApiV2 = PRIVATE_API_SPOT;
    }

    m_p->apiKey = apiKey;
    m_p->apiSecret = apiSecret;

    /// 2400 is the default value according to https://binance-docs.github.io/apidocs/futures/en/#limits
    m_p->weightLimit = 2400 * 0.85;
    spdlog::info(fmt::format("API Weight limit: {}", m_p->weightLimit));
}

HTTPSession::~HTTPSession() = default;

http::response<http::string_body> HTTPSession::get(const std::string &target, const bool isPublic) const {
    std::string finalTarget = target;

    if (!isPublic) {
        m_p->addTimestampToTargetPath(finalTarget);
    }

    std::string endpoint;

    if (isPublic) {
        endpoint = m_p->publicApi + finalTarget;
    } else {
        endpoint = m_p->privateApi + finalTarget;
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
        endpoint = m_p->publicApiV2 + finalTarget;
    } else {
        endpoint = m_p->privateApiV2 + finalTarget;
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
        endpoint = m_p->publicApi + finalTarget;
    } else {
        endpoint = m_p->privateApi + finalTarget;
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
        endpoint = m_p->publicApi + finalTarget;
    } else {
        endpoint = m_p->privateApi + finalTarget;
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
        endpoint = m_p->publicApi + finalTarget;
    } else {
        endpoint = m_p->privateApi + finalTarget;
    }

    const http::request<http::string_body> req{http::verb::delete_, endpoint, 11};
    return m_p->request(req);
}

http::response<http::string_body> HTTPSession::P::request(
    http::request<http::string_body> req) {
    req.set(http::field::host, uri);
    req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

    ssl::context ctx{ssl::context::sslv23_client};
    enableTlsPeerVerification(ctx);

    tcp::resolver resolver{ioc};
    ssl::stream<tcp::socket> stream{ioc, ctx};
    stream.set_verify_callback(ssl::host_name_verification(uri));

    // Set SNI Hostname (many hosts need this to handshake successfully)
    if (!SSL_set_tlsext_host_name(stream.native_handle(), uri.c_str())) {
        boost::system::error_code ec{
            static_cast<int>(ERR_get_error()),
            net::error::get_ssl_category()
        };
        throw boost::system::system_error{ec};
    }

    req.set("X-MBX-APIKEY", apiKey);

    if (req.method() == http::verb::post) {
        req.set(http::field::content_type, "application/json");
    }

    beast::flat_buffer buffer;
    http::response_parser<http::string_body> parser;
    parser.body_limit((std::numeric_limits<std::uint64_t>::max)());

    /// Everything below can fail without the exchange ever seeing the request - or after it has seen it. The caller
    /// must be able to tell that apart from a rejection, hence the dedicated exception type.
    try {
        auto const results = resolver.resolve(uri, "443");
        net::connect(stream.next_layer(), results.begin(), results.end());
        applySocketTimeout(stream.next_layer(), requestTimeoutMs);
        stream.handshake(ssl::stream_base::client);
        http::write(stream, req);
        http::read(stream, buffer, parser);
    } catch (const boost::system::system_error &e) {
        throw TransportError(fmt::format("Transport failure for {}: {}", std::string(req.target()), e.what()));
    }

    lastSuccessMs = getMsTimestamp(currentTime()).count();

    std::string limiterName("X-MBX-USED-WEIGHT-1M");

    for (auto &h : parser.get().base()) {
      if (std::ranges::equal(h.name_string(), limiterName, [](auto a, auto b) {
            return std::tolower(static_cast<unsigned char>(a)) ==
                   std::tolower(static_cast<unsigned char>(b));
          })) {
        usedWeight = std::stoi(std::string(h.value()));
      } else if (h.name_string() == "Date") {
        const auto dateString = std::string(h.value());
        std::string timeFormat = "%a, %d %b %Y %H:%M:%S";
        lastResponseTime = getTimeFromString(dateString, timeFormat);
      }
    }

    if (usedWeight >= weightLimit) {
        auto secToWeighReset = 60 - lastResponseTime.load().tm_sec;
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

    return parser.get();
}

void HTTPSession::P::ensureTimeSync() const {
    const auto now = getMsTimestamp(currentTime()).count();

    if (lastTimeSyncMs != 0 && now - lastTimeSyncMs < TIME_SYNC_INTERVAL_MS) {
        return;
    }

    /// Set upfront so that a failing endpoint is not hammered on every single signed request
    lastTimeSyncMs = now;

    try {
        /// Public endpoint - does not go through addTimestampToTargetPath, so this cannot recurse
        const auto response = parent->get("time?", true);

        if (response.result() != http::status::ok) {
            spdlog::warn(fmt::format("Time synchronization failed, HTTP {}", response.result_int()));
            return;
        }

        std::int64_t serverTime = 0;
        readValue<std::int64_t>(nlohmann::json::parse(response.body()), "serverTime", serverTime);

        if (serverTime <= 0) {
            return;
        }

        const auto offset = serverTime - getMsTimestamp(currentTime()).count();
        timeOffsetMs = offset;

        if (std::abs(offset) > 1000) {
            spdlog::warn(fmt::format("Local clock differs from the exchange clock by {} ms, compensating", offset));
        }
    } catch (const std::exception &e) {
        spdlog::warn(fmt::format("Time synchronization failed: {}", e.what()));
    }
}

void HTTPSession::P::addTimestampToTargetPath(std::string &target) const {
    ensureTimeSync();

    std::string parameters = target.substr(target.find('?') + 1);
    const std::string path = target.substr(0, target.find('?') + 1);

    parameters.append("&recvWindow=");
    parameters.append(std::to_string(RECV_WINDOW_MS));

    parameters.append("&timestamp=");
    parameters.append(std::to_string(getMsTimestamp(currentTime()).count() + timeOffsetMs.load()));

    unsigned char digest[SHA256_DIGEST_LENGTH];
    unsigned int digestLength = SHA256_DIGEST_LENGTH;

    HMAC(evpMd, apiSecret.data(), apiSecret.size(),
         reinterpret_cast<const unsigned char *>(parameters.data()),
         parameters.length(), digest, &digestLength);

    const std::string signature = stringToHex(digest, sizeof(digest));

    parameters.append("&signature=");
    parameters.append(signature);

    target = path + parameters;
}

void HTTPSession::setWeightLimit(const std::int32_t weightLimit) const {
    m_p->weightLimit = static_cast<std::int32_t>(weightLimit * 0.95);
}

std::int32_t HTTPSession::getUsedWeight() const {
    return m_p->usedWeight;
}

std::int64_t HTTPSession::lastSuccessfulResponseMs() const {
    return m_p->lastSuccessMs;
}

void HTTPSession::setRequestTimeout(const int timeoutMs) const {
    m_p->requestTimeoutMs = timeoutMs;
}
}
