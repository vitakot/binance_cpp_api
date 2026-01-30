/**
Binance Spot REST Client

Licensed under the MIT License <http://opensource.org/licenses/MIT>.
SPDX-License-Identifier: MIT
Copyright (c) 2022 Vitezslav Kot <vitezslav.kot@gmail.com>.
*/

#include "vk/binance/binance_spot_rest_client.h"
#include "vk/binance/binance_http_session.h"
#include "vk/utils/json_utils.h"
#include "vk/utils/utils.h"
#include <mutex>
#include <future>
#include <spdlog/spdlog.h>

namespace vk::binance::spot {
static constexpr std::int64_t EXCHANGE_DATA_MAX_AGE_S = 3600; /// 1 hour

enum class PrecisionType : int {
    Quantity,
    Price,
    Quote
};

struct RESTClient::P {
private:
    Exchange m_exchange;
    mutable std::recursive_mutex m_locker;

public:
    RESTClient *parent = nullptr;
    std::shared_ptr<HTTPSession> httpSession;

    [[nodiscard]] Exchange getExchange() const {
        std::lock_guard lk(m_locker);
        return m_exchange;
    }

    void setExchange(const Exchange &exchange) {
        std::lock_guard lk(m_locker);
        m_exchange = exchange;
    }

    [[nodiscard]] std::vector<Candle>
    getHistoricalPrices(const std::string &symbol, CandleInterval interval, std::int64_t startTime,
                        std::int64_t endTime, std::int32_t limit) const;

    explicit P(RESTClient *parent) {
        this->parent = parent;
    }
};

http::response<http::string_body> checkResponse(const http::response<http::string_body> &response) {
    if (response.result() != http::status::ok) {
        ErrorResponse errorResponse;
        errorResponse.fromJson(nlohmann::json::parse(response.body()));

        const std::string msg = std::string("Bad HTTP response: ") + std::to_string(response.result_int()) +
                                ", API Code: " +
                                std::to_string(errorResponse.code) + ", message: " + errorResponse.msg;
        throw std::runtime_error(msg.c_str());
    }
    return response;
}

RESTClient::RESTClient(const std::string &apiKey, const std::string &apiSecret) : m_p(
    std::make_unique<P>(this)) {
    m_p->httpSession = std::make_shared<HTTPSession>(apiKey, apiSecret, false);
}

RESTClient::~RESTClient() = default;

std::vector<Candle>
RESTClient::P::getHistoricalPrices(const std::string &symbol, const CandleInterval interval,
                                   const std::int64_t startTime,
                                   const std::int64_t endTime, const std::int32_t limit) const {
    std::string path = "klines?symbol=";
    path.append(symbol);

    path.append("&interval=");
    auto intervalStr = std::string(magic_enum::enum_name(interval));
    intervalStr.erase(0, 1);
    path.append(intervalStr);

    if (startTime != -1) {
        path.append("&startTime=");
        path.append(std::to_string(startTime));
    }

    if (endTime != -1) {
        path.append("&endTime=");
        path.append(std::to_string(endTime));
    }

    if (limit != -1) {
        path.append("&limit=");
        path.append(std::to_string(limit));
    }

    auto response = checkResponse(httpSession->get(path, true));
    CandlesResponse candlesResponse;
    candlesResponse.fromJson(nlohmann::json::parse(response.body()));

    return candlesResponse.candles;
}

Exchange RESTClient::getExchangeInfo(const bool force) const {
    if (m_p->getExchange().symbols.empty() || force) {
        const auto response = checkResponse(m_p->httpSession->get("exchangeInfo?", true));

        Exchange exchange;
        exchange.fromJson(nlohmann::json::parse(response.body()));
        exchange.lastUpdateTime = std::time(nullptr);
        m_p->setExchange(exchange);
    }

    return m_p->getExchange();
}

void RESTClient::setAPIWeightLimit(const std::int32_t weightLimit) const {
    m_p->httpSession->setWeightLimit(weightLimit);
}

std::vector<Candle>
RESTClient::getHistoricalPricesSingle(const std::string &symbol, const CandleInterval interval,
                                      const std::int64_t startTime,
                                      const std::int64_t endTime, const std::int32_t limit) const {
    std::string path = "klines?symbol=";
    path.append(symbol);

    path.append("&interval=");
    auto intervalStr = std::string(magic_enum::enum_name(interval));
    intervalStr.erase(0, 1);
    path.append(intervalStr);

    if (startTime != -1) {
        path.append("&startTime=");
        path.append(std::to_string(startTime));
    }

    if (endTime != -1) {
        path.append("&endTime=");
        path.append(std::to_string(endTime));
    }

    if (limit != -1) {
        path.append("&limit=");
        path.append(std::to_string(limit));
    }

    auto response = checkResponse(m_p->httpSession->get(path, true));
    CandlesResponse candlesResponse;
    candlesResponse.fromJson(nlohmann::json::parse(response.body()));

    return candlesResponse.candles;
}

std::vector<Candle>
RESTClient::getHistoricalPrices(const std::string &symbol, const CandleInterval interval, const std::int64_t startTime,
                                const std::int64_t endTime, const std::int32_t limit) const {
    std::vector<Candle> retVal;
    std::int64_t lastFromTime = startTime;
    std::vector<Candle> candles;

    if (lastFromTime < endTime) {
        candles = m_p->getHistoricalPrices(symbol, interval, lastFromTime, endTime, limit);
    }

    while (!candles.empty()) {
        retVal.insert(retVal.end(), candles.begin(), candles.end());
        lastFromTime = candles.back().closeTime;
        candles.clear();

        if (lastFromTime < endTime) {
            candles = m_p->getHistoricalPrices(symbol, interval, lastFromTime, endTime, limit);
        }
    }

    /// Remove last candle as it is invalid (not complete yet)
    if (!retVal.empty()) {
        retVal.pop_back();
    }

    return retVal;
}

std::map<std::string, std::vector<Candle> >
RESTClient::getHistoricalPrices(const std::vector<std::string> &symbols, CandleInterval candleInterval,
                                std::int64_t startTime,
                                std::int64_t endTime, std::int32_t limit) const {
    std::map<std::string, std::vector<Candle> > retVal;

    std::vector<std::future<std::pair<std::string, std::vector<Candle> > > > futures;

    for (const auto &symbol: symbols) {
        futures.push_back(
            std::async(std::launch::async, [this](const std::string &symbolIn,
                                                  const CandleInterval candleIntervalIn, const std::int64_t startTimeIn,
                                                  const std::int64_t endTimeIn,
                                                  const std::int32_t limitIn) -> std::pair<std::string, std::vector<
                   Candle> > {
                           std::pair lambdaRet(symbolIn,
                                               getHistoricalPrices(
                                                   symbolIn,
                                                   candleIntervalIn,
                                                   startTimeIn,
                                                   endTimeIn,
                                                   limitIn));
                           return lambdaRet;
                       }, symbol, candleInterval, startTime, endTime, limit));
    }

    for (auto &future: futures) {
        const auto [fst, snd] = future.get();
        retVal.insert_or_assign(fst, snd);
    }

    return retVal;
}
}
