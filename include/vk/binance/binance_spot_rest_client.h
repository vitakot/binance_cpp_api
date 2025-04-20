/**
Binance Spot REST Client

Licensed under the MIT License <http://opensource.org/licenses/MIT>.
SPDX-License-Identifier: MIT
Copyright (c) 2022 Vitezslav Kot <vitezslav.kot@gmail.com>.
*/

#ifndef INCLUDE_VK_BINANCE_SPOT_REST_CLIENT_H
#define INCLUDE_VK_BINANCE_SPOT_REST_CLIENT_H

#include <string>
#include <memory>
#include "binance_models.h"

namespace vk::binance::spot {
class RESTClient {
    struct P;
    std::unique_ptr<P> m_p{};

public:
    RESTClient(const std::string &apiKey, const std::string &apiSecret);

    ~RESTClient();

    /**
     * Get Exchange info
     * @param force Reload Exchange info if true
     * @return Exchange structure
     * @throws nlohmann::json::exception, std::exception
     */
    [[nodiscard]] Exchange getExchangeInfo(bool force = false) const;

    /**
     * Set maximal requests weight
     * @param weightLimit
     * @see https://developers.binance.com/docs/binance-spot-api-docs/rest-api/limits
     */
    void setAPIWeightLimit(std::int32_t weightLimit) const;

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
};
}
#endif //INCLUDE_VK_BINANCE_SPOT_REST_CLIENT_H
