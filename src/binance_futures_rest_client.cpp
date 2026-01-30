/**
Binance Futures REST Client

Licensed under the MIT License <http://opensource.org/licenses/MIT>.
SPDX-License-Identifier: MIT
Copyright (c) 2022 Vitezslav Kot <vitezslav.kot@gmail.com>.
*/

#include "vk/binance/binance_futures_rest_client.h"
#include "vk/binance/binance_http_session.h"
#include "vk/utils/json_utils.h"
#include "vk/utils/utils.h"
#include <mutex>
#include <future>
#include <spdlog/spdlog.h>

namespace vk::binance::futures {
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

    [[nodiscard]] std::vector<FundingRate>
    getFundingRates(const std::string &symbol, int64_t startTime, int64_t endTime,
                    int limit) const;

    int findPrecisionForSymbol(const PrecisionType &type, const std::string &symbol) const {
        if (getExchange().lastUpdateTime < 0 || std::time(nullptr) - getExchange().lastUpdateTime >
            EXCHANGE_DATA_MAX_AGE_S) {
            this->parent->updateExchangeInfo(true);
        }

        for (const auto &symbolEl: getExchange().symbols) {
            if (symbolEl.symbol == symbol) {
                switch (type) {
                    case PrecisionType::Quantity:
                        return symbolEl.quantityPrecision;
                    case PrecisionType::Price:
                        return symbolEl.pricePrecision;
                    case PrecisionType::Quote:
                        return symbolEl.quotePrecision;
                }
            }
        }
        return 1;
    }

    explicit P(RESTClient *parent) {
        this->parent = parent;
    }

    std::vector<OpenInterestStatistics>
    getOpenInterestStatistics(const std::string &symbol, StatisticsPeriod period, std::int64_t startTime,
                              std::int64_t endTime,
                              std::int32_t limit) const;

    std::vector<LongShortRatio>
    getLongShortRatio(const std::string &symbol, StatisticsPeriod period, std::int64_t startTime,
                      std::int64_t endTime,
                      std::int32_t limit) const;

    std::vector<BuySellVolume>
    getBuySellVolume(const std::string &symbol, StatisticsPeriod period, std::int64_t startTime,
                     std::int64_t endTime,
                     std::int32_t limit) const;
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
    m_p->httpSession = std::make_shared<HTTPSession>(apiKey, apiSecret, true);
}

RESTClient::~RESTClient() = default;

void RESTClient::setCredentials(const std::string &apiKey, const std::string &apiSecret) const {
    m_p->httpSession.reset();
    m_p->httpSession = std::make_shared<HTTPSession>(apiKey, apiSecret, true);
}

std::vector<FundingRate>
RESTClient::P::getFundingRates(const std::string &symbol, const int64_t startTime, const int64_t endTime,
                               const int limit) const {
    std::string path = "fundingRate?symbol=";
    path.append(symbol);

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

    const auto response = checkResponse(httpSession->get(path, true));
    FundingRates fundingRates;
    fundingRates.fromJson(nlohmann::json::parse(response.body()));
    return fundingRates.fundingRates;
}

FundingRate RESTClient::getLastFundingRate(const std::string &symbol) const {
    if (symbol.empty()) {
       throw std::runtime_error(std::string("Invalid parameter, symbol must be specified").c_str());
    }

    const auto response = checkResponse(m_p->httpSession->get("fundingRate?symbol=" + symbol, true));
    FundingRates fundingRates;
    fundingRates.fromJson(nlohmann::json::parse(response.body()));

    std::ranges::sort(fundingRates.fundingRates,
                      [](const FundingRate &a, const FundingRate &b) -> bool {
                          return a.fundingTime < b.fundingTime;
                      });

    return fundingRates.fundingRates.back();
}

MarkPrice RESTClient::getMarkPrice(const std::string &symbol) const {
    if (symbol.empty()) {
        throw std::runtime_error(std::string("Invalid parameter, symbol must be specified").c_str());
    }

    const auto response = checkResponse(m_p->httpSession->get("premiumIndex?symbol=" + symbol, true));
    MarkPrice markPrice;
    markPrice.fromJson(nlohmann::json::parse(response.body()));
    return markPrice;
}

TickerPrice RESTClient::getTickerPrice(const std::string &symbol) const {
    if (symbol.empty()) {
        throw std::runtime_error(std::string("Invalid parameter, symbol must be specified").c_str());
    }

    const auto response = checkResponse(m_p->httpSession->get("ticker/price?symbol=" + symbol, true));
    TickerPrice tickerPrice;
    tickerPrice.fromJson(nlohmann::json::parse(response.body()));
    return tickerPrice;
}

BookTickerPrice RESTClient::getBookTickerPrice(const std::string &symbol) const {
    if (symbol.empty()) {
        throw std::runtime_error(std::string("Invalid parameter, symbol must be specified").c_str());
    }

    const auto response = checkResponse(m_p->httpSession->get("ticker/bookTicker?symbol=" + symbol, true));
    BookTickerPrice bookTickerPrice;
    bookTickerPrice.fromJson(nlohmann::json::parse(response.body()));
    return bookTickerPrice;
}

std::vector<MarkPrice> RESTClient::getMarkPrices() const {
    const auto response = checkResponse(m_p->httpSession->get("premiumIndex", true));
    MarkPrices markPrices;
    markPrices.fromJson(nlohmann::json::parse(response.body()));
    return markPrices.markPrices;
}

OrderResponse RESTClient::sendOrder(const Order &order) const {
    auto quantityPrecision = m_p->findPrecisionForSymbol(PrecisionType::Quantity, order.symbol);
    auto pricePrecision = m_p->findPrecisionForSymbol(PrecisionType::Price, order.symbol);

    std::string path = "order?symbol=";
    path.append(order.symbol);

    path.append("&side=");
    path.append(magic_enum::enum_name(order.side));

    path.append("&positionSide=");
    path.append(magic_enum::enum_name(order.positionSide));

    path.append("&type=");
    path.append(magic_enum::enum_name(order.type));

    if (order.type == OrderType::LIMIT) {
        path.append("&timeInForce=");
        path.append(magic_enum::enum_name(order.timeInForce));

        path.append("&quantity=");
        path.append(formatDouble(quantityPrecision, order.quantity));

        path.append("&price=");
        path.append(formatDouble(pricePrecision, order.price));
    } else if (order.type == OrderType::MARKET) {
        path.append("&quantity=");
        path.append(formatDouble(quantityPrecision, order.quantity));
    } else if (order.type == OrderType::STOP ||
               order.type == OrderType::TAKE_PROFIT) {
        path.append("&quantity=");
        path.append(formatDouble(quantityPrecision, order.quantity));

        path.append("&price=");
        path.append(formatDouble(pricePrecision, order.price));

        path.append("&stopPrice=");
        path.append(formatDouble(pricePrecision, order.stopPrice));
    } else if (order.type == OrderType::STOP_MARKET ||
               order.type == OrderType::TAKE_PROFIT_MARKET) {
        path.append("&quantity=");
        path.append(formatDouble(quantityPrecision, order.quantity));

        path.append("&stopPrice=");
        path.append(formatDouble(pricePrecision, order.stopPrice));
    } else if (order.type == OrderType::TRAILING_STOP_MARKET) {
        path.append("&quantity=");
        path.append(formatDouble(quantityPrecision, order.quantity));

        path.append("&callbackRate=");
        path.append(std::to_string(order.callbackRate));

        path.append("&activationPrice=");
        path.append(formatDouble(pricePrecision, order.activationPrice));
    }

    if (order.positionSide == PositionSide::BOTH) {
        path.append("&reduceOnly=");
        path.append(fmt::format("{}", order.reduceOnly));
    }

    if (!order.newClientOrderId.empty()) {
        path.append("&newClientOrderId=");
        path.append(order.newClientOrderId);
    }

    path.append("&newOrderRespType=");
    path.append(magic_enum::enum_name(order.newOrderRespType));

    const auto response = checkResponse(m_p->httpSession->post(path, "", false));
    OrderResponse retVal;
    retVal.fromJson(nlohmann::json::parse(response.body()));
    return retVal;
}

Account RESTClient::getAccountInfo() const {
    const auto response = checkResponse(m_p->httpSession->getV2("account?", false));
    Account account;
    account.fromJson(nlohmann::json::parse(response.body()));
    return account;
}

std::int64_t RESTClient::getServerTime() const {
    const auto response = checkResponse(m_p->httpSession->get("time?", true));
    std::int64_t time;
    readValue<std::int64_t>(nlohmann::json::parse(response.body()), "serverTime", time);
    return time;
}

std::string RESTClient::startUserDataStream() const {
    const auto response = checkResponse(m_p->httpSession->post("listenKey?", "", false));
    std::string listenKey;
    readValue<std::string>(nlohmann::json::parse(response.body()), "listenKey", listenKey);
    return listenKey;
}

void RESTClient::keepAliveUserDataStream() const {
    const auto response = checkResponse(m_p->httpSession->put("listenKey?", "", false));
}

void RESTClient::closeUserDataStream() const {
    const auto response = checkResponse(m_p->httpSession->del("listenKey?", false));
}

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

PositionMode RESTClient::getPositionMode() const {
    const auto response = checkResponse(m_p->httpSession->get("positionSide/dual?", false));

    bool isDualMode;
    readValue<bool>(nlohmann::json::parse(response.body()), "dualSidePosition", isDualMode);

    if (isDualMode) {
        return PositionMode::Hedge;
    }

    return PositionMode::OneWay;
}

OrderResponse
RESTClient::cancelOrder(const std::string &symbol, const std::string &clientId, std::int64_t orderId) const {
    std::string path = "order?symbol=";
    path.append(symbol);

    if (!clientId.empty()) {
        path.append("&origClientOrderId=");
        path.append(clientId);
    }

    if (orderId != 0) {
        path.append("&orderId=");
        path.append(std::to_string(orderId));
    }

    const auto response = checkResponse(m_p->httpSession->del(path, false));
    OrderResponse retVal;
    retVal.fromJson(nlohmann::json::parse(response.body()));
    return retVal;
}

OrderResponse
RESTClient::queryOrder(const std::string &symbol, const std::string &clientId, std::int64_t orderId) const {
    std::string path = "order?symbol=";
    path.append(symbol);

    if (!clientId.empty()) {
        path.append("&origClientOrderId=");
        path.append(clientId);
    }

    if (orderId != 0) {
        path.append("&orderId=");
        path.append(std::to_string(orderId));
    }

    const auto response = checkResponse(m_p->httpSession->get(path, false));
    OrderResponse retVal;
    retVal.fromJson(nlohmann::json::parse(response.body()));
    return retVal;
}

std::vector<Position> RESTClient::getPosition(const std::string &symbol) const {
    std::string path = "positionRisk?";

    if (!symbol.empty()) {
        path.append("&symbol=");
        path.append(symbol);
    }

    const auto response = checkResponse(m_p->httpSession->getV2(path, false));
    std::vector<Position> retVal;

    for (nlohmann::json jsonObject = nlohmann::json::parse(response.body()); const auto &el: jsonObject) {
        Position position;
        position.fromJson(el);
        retVal.push_back(position);
    }

    return retVal;
}

Exchange RESTClient::getExchangeInfo(const bool force) const {
    updateExchangeInfo(force);
    return m_p->getExchange();
}

void RESTClient::updateExchangeInfo(bool force) const {

    if (m_p->getExchange().lastUpdateTime < 0 || std::time(nullptr) - m_p->getExchange().lastUpdateTime > EXCHANGE_DATA_MAX_AGE_S) {
        force = true;
    }

    if (m_p->getExchange().symbols.empty() || force) {
        const auto response = checkResponse(m_p->httpSession->get("exchangeInfo?", true));

        Exchange exchange;
        exchange.fromJson(nlohmann::json::parse(response.body()));
        exchange.lastUpdateTime = std::time(nullptr);
        m_p->setExchange(exchange);
    }
}

std::vector<AccountBalance> RESTClient::getAccountBalances() const {
    std::vector<AccountBalance> retVal;
    const auto response = checkResponse(m_p->httpSession->getV2("balance?", false));

    for (nlohmann::json balancesObj = nlohmann::json::parse(response.body()); const auto &el: balancesObj) {
        AccountBalance accountBalance;
        accountBalance.fromJson(el);
        retVal.push_back(accountBalance);
    }
    return retVal;
}

std::vector<Order> RESTClient::getAllOpenOrders(const std::string &symbol) const {
    std::string path = "openOrders?symbol=";
    path.append(symbol);

    const auto response = checkResponse(m_p->httpSession->get(path, false));
    std::vector<Order> retVal;

    for (nlohmann::json jsonObject = nlohmann::json::parse(response.body()); const auto &el: jsonObject) {
        Order order;
        order.fromJson(el);
        retVal.push_back(order);
    }

    return retVal;
}

bool RESTClient::cancelAllOpenOrders(const std::string &symbol, std::string &errorMsg) const {
    std::string path = "allOpenOrders?symbol=";
    path.append(symbol);

    const auto response = checkResponse(m_p->httpSession->del(path, false));

    nlohmann::json jsonObject = nlohmann::json::parse(response.body());

    if (jsonObject["code"] == 200) {
        return true;
    }

    errorMsg = jsonObject["msg"];

    return false;
}

std::vector<OrderResponse> RESTClient::sendOrders(std::vector<Order> &orders) const {
    std::string path = "batchOrders?batchOrders=";

    nlohmann::json ordersJson = nlohmann::json::array();

    for (auto &order: orders) {
        order.quantityPrecision = m_p->findPrecisionForSymbol(PrecisionType::Quantity, order.symbol);
        order.pricePrecision = m_p->findPrecisionForSymbol(PrecisionType::Price, order.symbol);
        ordersJson.push_back(order.toJson());
    }

    const std::string ordersStr = ordersJson.dump();
    path.append(ordersStr);

    const auto response = checkResponse(m_p->httpSession->post(path, "", false));
    OrdersResponse ordersResponse;
    ordersResponse.fromJson(nlohmann::json::parse(response.body()));
    return ordersResponse.responses;
}

DownloadId RESTClient::getDownloadId(const std::int64_t startTime, const std::int64_t endTime) const {
    std::string path = "income/asyn?";

    path.append("&startTime=");
    path.append(std::to_string(startTime));

    path.append("&endTime=");
    path.append(std::to_string(endTime));

    const auto response = checkResponse(m_p->httpSession->get(path, false));
    DownloadId downloadId;
    downloadId.fromJson(nlohmann::json::parse(response.body()));
    return downloadId;
}

std::string RESTClient::getDownloadUrl(const DownloadId &downloadId) const {
    std::string path = "income/asyn/id?";

    path.append("&downloadId=");
    path.append(downloadId.downloadId);

    const auto response = checkResponse(m_p->httpSession->get(path, false));

    nlohmann::json jsonObject = nlohmann::json::parse(response.body());
    return jsonObject["url"];
}

std::vector<Income> RESTClient::getIncome(const std::string &symbol, const std::int64_t startTime,
                                          const std::int64_t endTime,
                                          const IncomeType incomeType) const {
    std::string path = "income?";

    if (!symbol.empty()) {
        path.append("&symbol=");
        path.append(symbol);
    }

    if (incomeType != IncomeType::ALL) {
        path.append("&incomeType=");
        path.append(symbol);
    }

    if (startTime != -1) {
        path.append("&startTime=");
        path.append(std::to_string(startTime));
    }

    if (endTime != -1) {
        path.append("&endTime=");
        path.append(std::to_string(endTime));
    }

    const auto response = checkResponse(m_p->httpSession->get(path, false));
    Incomes incomes;
    incomes.fromJson(nlohmann::json::parse(response.body()));
    return incomes.incomes;
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

std::vector<PositionRisk> RESTClient::getPositionRisk(const std::string &symbol) const {
    std::string path = "positionRisk?symbol=";
    path.append(symbol);

    const auto response = checkResponse(m_p->httpSession->get(path, false));
    std::vector<PositionRisk> retVal;

    for (nlohmann::json jsonObject = nlohmann::json::parse(response.body()); const auto &el: jsonObject) {
        PositionRisk positionRisk;
        positionRisk.fromJson(el);
        retVal.push_back(positionRisk);
    }

    return retVal;
}

std::int32_t RESTClient::getUsedAPIWeight() const {
    return m_p->httpSession->getUsedWeight();
}

void RESTClient::setAPIWeightLimit(const std::int32_t weightLimit) const {
    m_p->httpSession->setWeightLimit(weightLimit);
}

void RESTClient::setExchangeInfo(const Exchange &exchange) const {
    m_p->setExchange(exchange);
}

std::pair<int, double> RESTClient::changeInitialLeverage(const std::string &symbol, const int leverage) const {
    std::string path = "leverage?symbol=";
    path.append(symbol);
    path.append("&leverage=");
    path.append(std::to_string(leverage));

    const auto response = checkResponse(m_p->httpSession->post(path, "", false));
    const nlohmann::json responseJson = nlohmann::json::parse(response.body());

    int targetLeverage;
    std::string maxNotionalValue;
    readValue<int>(responseJson, "leverage", targetLeverage);
    readValue<std::string>(responseJson, "maxNotionalValue", maxNotionalValue);

    return {targetLeverage, stod(maxNotionalValue)};
}

std::vector<FundingRate>
RESTClient::getFundingRates(const std::string &symbol, const std::int64_t startTime, const std::int64_t endTime,
                            const std::int32_t limit) const {
    std::vector<FundingRate> retVal;
    std::int64_t lastFromTime = startTime;
    std::vector<FundingRate> fr;

    if (lastFromTime < endTime) {
        fr = m_p->getFundingRates(symbol, lastFromTime, endTime, limit);
    }

    while (!fr.empty()) {
        retVal.insert(retVal.end(), fr.begin(), fr.end());
        lastFromTime = fr.back().fundingTime;
        fr.clear();

        if (lastFromTime < endTime) {
            fr = m_p->getFundingRates(symbol, lastFromTime, endTime, limit);

            if (fr.size() == 1) {
                if (lastFromTime == fr.back().fundingTime) {
                    break;
                }
            }
        }
    }

    return retVal;
}

OpenInterest RESTClient::getOpenInterest(const std::string &symbol) const {
    std::string path = "openInterest?symbol=";
    path.append(symbol);

    const auto response = checkResponse(m_p->httpSession->get(path, true));
    OpenInterest retVal;
    retVal.fromJson(nlohmann::json::parse(response.body()));
    return retVal;
}

std::vector<OpenInterestStatistics>
RESTClient::P::getOpenInterestStatistics(const std::string &symbol, const StatisticsPeriod period,
                                         const std::int64_t startTime,
                                         const std::int64_t endTime,
                                         const std::int32_t limit) const {
    std::vector<OpenInterestStatistics> retVal;

    try {
        std::string path = "/futures/data/openInterestHist?symbol=";
        path.append(symbol);

        path.append("&period=");
        auto periodStr = std::string(magic_enum::enum_name(period));
        periodStr.erase(0, 1);
        path.append(periodStr);

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

        const auto response = checkResponse(httpSession->getFutures(path));

        for (nlohmann::json jsonObject = nlohmann::json::parse(response.body()); const auto &el: jsonObject) {
            OpenInterestStatistics openInterestStatistics;
            openInterestStatistics.fromJson(el);
            retVal.push_back(openInterestStatistics);
        }
    } catch (const std::exception &e) {
        spdlog::warn(fmt::format("Exception: {}", e.what()));
    }
    return retVal;
}

std::vector<OpenInterestStatistics>
RESTClient::getOpenInterestStatistics(const std::string &symbol, const StatisticsPeriod period,
                                      std::int64_t startTime) const {
    std::vector<OpenInterestStatistics> retVal;
    std::vector<OpenInterestStatistics> statistics = m_p->getOpenInterestStatistics(symbol, period, -1, startTime, 500);

    while (!statistics.empty()) {
        retVal.insert(retVal.begin(), statistics.begin(), statistics.end());
        startTime = statistics.front().timestamp - 1;
        statistics.clear();
        statistics = m_p->getOpenInterestStatistics(symbol, period, -1, startTime, 500);
    }

    return retVal;
}

std::vector<LongShortRatio>
RESTClient::P::getLongShortRatio(const std::string &symbol, const StatisticsPeriod period, const std::int64_t startTime,
                                 const std::int64_t endTime,
                                 const std::int32_t limit) const {
    std::vector<LongShortRatio> retVal;

    try {
        std::string path = "/futures/data/globalLongShortAccountRatio?symbol=";
        path.append(symbol);

        path.append("&period=");
        auto periodStr = std::string(magic_enum::enum_name(period));
        periodStr.erase(0, 1);
        path.append(periodStr);

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

        const auto response = checkResponse(httpSession->getFutures(path));

        for (nlohmann::json jsonObject = nlohmann::json::parse(response.body()); const auto &el: jsonObject) {
            LongShortRatio longShortRatio;
            longShortRatio.fromJson(el);
            retVal.push_back(longShortRatio);
        }
    } catch (const std::exception &e) {
        spdlog::warn(fmt::format("Exception: {}", e.what()));
    }

    return retVal;
}

std::vector<LongShortRatio>
RESTClient::getLongShortRatio(const std::string &symbol, const StatisticsPeriod period, std::int64_t startTime) const {
    std::vector<LongShortRatio> retVal;
    std::vector<LongShortRatio> ratio = m_p->getLongShortRatio(symbol, period, -1, startTime, 500);

    while (!ratio.empty()) {
        retVal.insert(retVal.begin(), ratio.begin(), ratio.end());
        startTime = ratio.front().timestamp - 1;
        ratio.clear();
        ratio = m_p->getLongShortRatio(symbol, period, -1, startTime, 500);
    }

    return retVal;
}

std::vector<BuySellVolume>
RESTClient::P::getBuySellVolume(const std::string &symbol, const StatisticsPeriod period, const std::int64_t startTime,
                                const std::int64_t endTime,
                                const std::int32_t limit) const {
    std::vector<BuySellVolume> retVal;

    try {
        std::string path = "/futures/data/takerlongshortRatio?symbol=";
        path.append(symbol);

        path.append("&period=");
        auto periodStr = std::string(magic_enum::enum_name(period));
        periodStr.erase(0, 1);
        path.append(periodStr);

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

        const auto response = checkResponse(httpSession->getFutures(path));

        for (nlohmann::json jsonObject = nlohmann::json::parse(response.body()); const auto &el: jsonObject) {
            BuySellVolume buySellVolume;
            buySellVolume.fromJson(el);
            retVal.push_back(buySellVolume);
        }
    } catch (const std::exception &e) {
        spdlog::warn(fmt::format("Exception: {}", e.what()));
    }

    return retVal;
}

std::vector<BuySellVolume>
RESTClient::getBuySellVolume(const std::string &symbol, const StatisticsPeriod period, std::int64_t startTime) const {
    std::vector<BuySellVolume> retVal;
    std::vector<BuySellVolume> bsVolume = m_p->getBuySellVolume(symbol, period, -1, startTime, 500);

    while (!bsVolume.empty()) {
        retVal.insert(retVal.begin(), bsVolume.begin(), bsVolume.end());
        startTime = bsVolume.front().timestamp - 1;
        bsVolume.clear();
        bsVolume = m_p->getBuySellVolume(symbol, period, -1, startTime, 500);
    }

    return retVal;
}
}
