/**
Binance Futures REST Client

Licensed under the MIT License <http://opensource.org/licenses/MIT>.
SPDX-License-Identifier: MIT
Copyright (c) 2022 Vitezslav Kot <vitezslav.kot@gmail.com>.
*/

#ifndef INCLUDE_VK_BINANCE_FUTURES_REST_CLIENT_H
#define INCLUDE_VK_BINANCE_FUTURES_REST_CLIENT_H

#include <string>
#include <memory>
#include "binance_models.h"

namespace vk::binance::futures {
class RESTClient {
    struct P;
    std::unique_ptr<P> m_p{};

public:
    RESTClient(const std::string &apiKey, const std::string &apiSecret);

    ~RESTClient();

    void setCredentials(const std::string &apiKey, const std::string &apiSecret) const;

    /**
     * If symbol is set then returns a vector of funding rates for it. If symbol is empty then returns funding rates
     * for all symbols.
     * If startTime and endTime are not sent, the most recent limit data are returned.
     * If the number of data between startTime and endTime is larger than limit, return as startTime + limit
     * @param symbol
     * @param startTime timestamp in ms to get funding rate from INCLUSIVE.
     * @param endTime timestamp in ms to get funding rate until INCLUSIVE.
     * @param limit max number of records, default 100; max 1000
     * @return vector of FundingRate structures
     * @throws nlohmann::json::exception, std::exception
     */
    [[nodiscard]] std::vector<FundingRate>
    getFundingRates(const std::string &symbol, int64_t startTime = -1, int64_t endTime = -1, int limit = -1) const;

    /**
     * Returns the most recent funding rate for a specified symbol.
     * @param symbol must not be empty
     * @return Filled FundingRate structure
     * @throws nlohmann::json::exception, std::exception
     */
    [[nodiscard]] FundingRate getLastFundingRate(const std::string &symbol) const;

    /**
     * Returns Mark Price and Funding Rate for a specified symbol
     * @param symbol must not be empty
     * @return Filled MarkPrice structure
     * @throws nlohmann::json::exception, std::exception
     */
    [[nodiscard]] MarkPrice getMarkPrice(const std::string &symbol) const;

    /**
     * Returns Ticker price for a specified symbol
     * @param symbol must not be empty
     * @return Filled TickerPrice structure
     * @throws nlohmann::json::exception, std::exception
     */
    [[nodiscard]] TickerPrice getTickerPrice(const std::string &symbol) const;

    /**
     * Returns Book Ticker price for a specified symbol
     * @param symbol must not be empty
     * @return Filled BookTickerPrice structure
     * @throws nlohmann::json::exception, std::exception
     */
    [[nodiscard]] BookTickerPrice getBookTickerPrice(const std::string &symbol) const;

    /**
     * Returns Mark Price and Funding Rate for Binance futures symbols
     * @return vector of MarkPrice structure
     * @throws nlohmann::json::exception, std::exception
     */
    [[nodiscard]] std::vector<MarkPrice> getMarkPrices() const;

    /**
     * Returns Account information
     * @return Filled Account structure
     * @throws nlohmann::json::exception, std::exception
     */
    [[nodiscard]] Account getAccountInfo() const;

    /**
     * Returns server time in ms
     * @return timestamp in ms
     * @throws nlohmann::json::exception, std::exception
     */
    [[nodiscard]] std::int64_t getServerTime() const;

    /**
     * Send order
     * @param order
     * @return Filled OrderResponse structure
     * @throws nlohmann::json::exception, std::exception
     */
    [[nodiscard]] OrderResponse sendOrder(const Order &order) const;

    /**
     * Query order - ask about Order that was already sent
     * @param symbol
     * @param clientId
     * @param orderId
     * @return Filled OrderResponse structure
     * @throws nlohmann::json::exception, std::exception
     */
    [[nodiscard]] OrderResponse
    queryOrder(const std::string &symbol, const std::string &clientId, std::int64_t orderId = 0) const;

    /**
     * Start User Data Stream and return its listenKey
     * @return listenKey
     * @throws nlohmann::json::exception, std::exception
     */
    [[nodiscard]] std::string startUserDataStream() const;

    /**
     * User Data Stream closes automatically after 60 minutes, send previously obtained listenKey to keep it
     * alive for another 60 minutes
     * @throws nlohmann::json::exception, std::exception
     */
    void keepAliveUserDataStream() const;

    /**
     * Close User Data Stream
     * @throws nlohmann::json::exception, std::exception
     */
    void closeUserDataStream() const;

    /**
     * Download historical candles
     * @param symbol e,g BTCUSDT
     * @param interval
     * @param startTime timestamp in ms, must be smaller than "endTime"
     * @param endTime timestamp in ms, must be greater than "startTime"
     * @param limit maximum number of returned candles, if set to -1 then it is ignored
     * @return vector of candles
     * @throws nlohmann::json::exception, std::exception
     */
    [[nodiscard]] std::vector<Candle>
    getHistoricalPrices(const std::string &symbol, CandleInterval interval, std::int64_t startTime,
                        std::int64_t endTime, std::int32_t limit = -1) const;

    /**
     * Download historical candles - simple BNB API method wrapper, returns max "limit" records.
     * @param symbol e,g BTCUSDT
     * @param interval
     * @param startTime timestamp in ms, must be smaller then "endTime"
     * @param endTime timestamp in ms, must be greater then "startTime"
     * @param limit maximum number of returned candles, if set to -1 then it is ignored
     * @return vector of candles
     * @throws nlohmann::json::exception, std::exception
     */
    [[nodiscard]] std::vector<Candle>
    getHistoricalPricesSingle(const std::string &symbol, CandleInterval interval, std::int64_t startTime,
                              std::int64_t endTime, std::int32_t limit) const;

    /**
     * Get Position Mode
     * @return PositionMode structure
     * @throws nlohmann::json::exception, std::exception
     */
    [[nodiscard]] PositionMode getPositionMode() const;

    /**
     * Cancel order
     * @param symbol e.g. BTCUSDT
     * @param clientId
     * @param orderId
     * @return OrderResponse structure
     * @throws nlohmann::json::exception, std::exception
     */
    [[nodiscard]] OrderResponse
    cancelOrder(const std::string &symbol, const std::string &clientId, std::int64_t orderId = 0) const;

    /**
     * Get position info - if Hedge mode is enabled then there is more than one Position
     * @param symbol e.g. BTCUSDT, if empty then positions of all symbols are returned.
     * @return vector of Position structures
     * @throws nlohmann::json::exception, std::exception
     */
    [[nodiscard]] std::vector<Position> getPosition(const std::string &symbol) const;

    /**
     * Get Exchange info
     * @param force Reload Exchange info if true
     * @return Exchange structure
     * @throws nlohmann::json::exception, std::exception
     */
    [[nodiscard]] Exchange getExchangeInfo(bool force = false) const;

    /**
     * Update Exchange info
     * @param force Reload Exchange info if true
     * @throws nlohmann::json::exception, std::exception
     */
    void updateExchangeInfo(bool force = false) const;

    /**
     * Returns AccountBalance information
     * @return vector of filled AccountBalance structures
     * @throws nlohmann::json::exception, std::exception
     */
    [[nodiscard]] std::vector<AccountBalance> getAccountBalances() const;

    /**
     * Returns all open orders for a given symbol
     * @param symbol e.g. BTCUSDT
     * @return vector of filled Order structures
     * @throws nlohmann::json::exception, std::exception
     */
    [[nodiscard]] std::vector<Order> getAllOpenOrders(const std::string &symbol) const;

    /**
     * Cancel all open orders for a given symbol
     * @param symbol e.g. BTCUSDT
     * @param errorMsg
     * @return True if successful
     * @throws nlohmann::json::exception, std::exception
     */
    [[nodiscard]] bool cancelAllOpenOrders(const std::string &symbol, std::string &errorMsg) const;

    /**
     * Send multiple orders as a batch
     * @param orders
     * @return vector of filled OrderResponse structures
     * @throws nlohmann::json::exception, std::exception
     */
    std::vector<OrderResponse> sendOrders(std::vector<Order> &orders) const;

    /**
     * Get Download Id For Futures Transaction History. Request Limitation is 5 times per month, shared by front end
     * download page and rest api
     * @param startTime timestamp in ms, must be smaller then "endTime"
     * @param endTime timestamp in ms, must be greater then "startTime"
     * @return DownloadId structure
     * @throws nlohmann::json::exception, std::exception
     */
    [[nodiscard]] DownloadId getDownloadId(std::int64_t startTime, std::int64_t endTime) const;

    /**
     * Get Futures Transaction History Download Link by Id, download link expiration: 24h
     * @param downloadId
     * @return URL
     * @throws nlohmann::json::exception, std::exception
     */
    [[nodiscard]] std::string getDownloadUrl(const DownloadId &downloadId) const;

    /**
     * Get Income History
     * @param symbol e.g. BTCUSDT
     * @param startTime timestamp in ms, must be smaller then "endTime"
     * @param endTime timestamp in ms, must be greater then "startTime"
     * @param incomeType see IncomeType
     * @return vector of filled Income structures
     * @throws nlohmann::json::exception, std::exception
     */
    [[nodiscard]] std::vector<Income>
    getIncome(const std::string &symbol = "", std::int64_t startTime = -1, std::int64_t endTime = -1,
              IncomeType incomeType = IncomeType::ALL) const;

    /**
     * Download historical candles for multiple symbols at once using parallel execution
     * @param symbols
     * @param candleInterval
     * @param startTime timestamp in ms, must be smaller then "endTime"
     * @param endTime timestamp in ms, must be greater then "startTime"
     * @param limit maximum number of returned candles, if set to -1 then it is ignored
     * @return map of vector of candles, map keys are symbols
     * @throws nlohmann::json::exception, std::exception
     */
    [[nodiscard]] std::map<std::string, std::vector<Candle> >
    getHistoricalPrices(const std::vector<std::string> &symbols, CandleInterval candleInterval, std::int64_t startTime,
                        std::int64_t endTime, std::int32_t limit = -1) const;

    /**
     * Get Position risk
     * @param symbol e.g. BTCUSDT
     * @return vector of filled PositionRisk structures (two structures for Hedge mode, one for One-way)
     */
    [[nodiscard]] std::vector<PositionRisk> getPositionRisk(const std::string &symbol) const;

    /**
     * Get currently used total requests weight
     * @return weight
     * @see https://binance-docs.github.io/apidocs/futures/en/#limits
     */
    [[nodiscard]] std::int32_t getUsedAPIWeight() const;

    /**
     * Set maximal requests weight
     * @param weightLimit
     * @see https://binance-docs.github.io/apidocs/futures/en/#limits
     */
    void setAPIWeightLimit(std::int32_t weightLimit) const;

    /**
     * Set exchange info
     * @param exchange
     */
    void setExchangeInfo(const Exchange &exchange) const;

    /**
     * Change user's initial leverage of specific symbol market
     * @param symbol e.g. BTCUSDT
     * @param leverage target initial leverage: int from 1 to 125
     * @return
     * @see https://binance-docs.github.io/apidocs/futures/en/#change-initial-leverage-trade
     */
    [[nodiscard]] std::pair<int, double> changeInitialLeverage(const std::string &symbol, int leverage) const;

    /**
     * Get present open interest of a specific symbol.
     * @param symbol e.g. BTCUSDT
     * @return filled OpenInterest structures
     */
    [[nodiscard]] OpenInterest getOpenInterest(const std::string &symbol) const;

    /**
     * Get open interest statistics. Only the data of the latest 30 days is available.
     * @param symbol  e.g. BTCUSDT
     * @param period
     * @param startTime timestamp in ms (returned values are older than that date)
     * @return vector of filled OpenInterestStatistics structures
     */
    [[nodiscard]] std::vector<OpenInterestStatistics>
    getOpenInterestStatistics(const std::string &symbol, StatisticsPeriod period, std::int64_t startTime) const;

    /**
     * Get Long/Short Ratio. Only the data of the latest 30 days is available.
     * @param symbol e.g. BTCUSDT
     * @param period
     * @param startTime timestamp in ms (returned values are older than that date)
     * @return vector of filled LongShortRatio structures
     */
    [[nodiscard]] std::vector<LongShortRatio>
    getLongShortRatio(const std::string &symbol, StatisticsPeriod period, std::int64_t startTime) const;

    /**
     * Get
     * @param symbol e.g. BTCUSDT
     * @param period
     * @param startTime timestamp in ms (returned values are older than that date)
     * @return vector of filled BuySellVolume structures
     */
    [[nodiscard]] std::vector<BuySellVolume>
    getBuySellVolume(const std::string &symbol, StatisticsPeriod period, std::int64_t startTime) const;
};
}
#endif //INCLUDE_VK_BINANCE_FUTURES_REST_CLIENT_H
