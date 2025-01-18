/**
Binance Data Models

Licensed under the MIT License <http://opensource.org/licenses/MIT>.
SPDX-License-Identifier: MIT
Copyright (c) 2022 Vitezslav Kot <vitezslav.kot@gmail.com>.
*/

#ifndef INCLUDE_CK_BINANCE_MODELS_H
#define INCLUDE_CK_BINANCE_MODELS_H

#include "vk/interface/i_json.h"
#include <nlohmann/json.hpp>

namespace vk::binance {
enum class CandleInterval : std::int32_t {
    _1m,
    _3m,
    _5m,
    _15m,
    _30m,
    _1h,
    _2h,
    _4h,
    _6h,
    _8h,
    _12h,
    _1d,
    _3d,
    _1w,
    _1M
};

enum class Side : std::int32_t {
    SELL,
    BUY
};

enum class PositionMode : std::int32_t {
    Hedge,
    OneWay
};

enum class ExecutionType : std::int32_t {
    NEW,
    CANCELED,
    CALCULATED,
    EXPIRED,
    TRADE
};

/**
 * GTC - Good till cancelled
 * IOC  # Immediate or cancel
 * FOK  # Fill or kill
 */
enum class TimeInForce : std::int32_t {
    GTC,
    IOC,
    FOK
};

enum class OrderRespType : std::int32_t {
    ACK,
    RESULT
};

enum class WorkingType : std::int32_t {
    MARK_PRICE,
    CONTRACT_PRICE
};
}

namespace vk::binance::futures {
enum class OrderStatus : std::int32_t {
    NEW,
    PARTIALLY_FILLED,
    FILLED,
    CANCELED,
    PENDING_CANCEL,
    REJECTED,
    EXPIRED,
    EXPIRED_IN_MATCH
};

enum class OrderType : std::int32_t {
    LIMIT,
    MARKET,
    STOP,
    STOP_MARKET,
    TAKE_PROFIT,
    TAKE_PROFIT_MARKET,
    TRAILING_STOP_MARKET,
    LIMIT_MAKER
};


/**
 * Default BOTH for One-way Mode ; LONG or SHORT for Hedge Mode. It must be sent in Hedge Mode.
 */

enum class PositionSide : std::int32_t {
    BOTH,
    LONG,
    SHORT
};

/**
 * stopPrice triggered by: "MARK_PRICE", "CONTRACT_PRICE". Default "CONTRACT_PRICE"
 */
enum class WorkingType : std::int32_t {
    REQUEST_WEIGHT,
    CONTRACT_PRICE,
    MARK_PRICE
};

enum class RateLimitType : std::int32_t {
    RAW_REQUEST,
    ORDERS,
    REQUEST_WEIGHT
};

enum class RateLimitInterval : std::int32_t {
    MONTH,
    WEEK,
    DAY,
    HOUR,
    MINUTE,
    SECOND
};

enum class SymbolFilter : std::int32_t {
    PRICE_FILTER,
    LOT_SIZE,
    MARKET_LOT_SIZE,
    MAX_NUM_ORDERS,
    MAX_NUM_ALGO_ORDERS,
    PERCENT_PRICE,
    MIN_NOTIONAL
};

enum class ContractType : std::int32_t {
    PERPETUAL,
    CURRENT_MONTH,
    NEXT_MONTH,
    CURRENT_QUARTER,
    NEXT_QUARTER
};

enum class ContractStatus : std::int32_t {
    PENDING_TRADING,
    TRADING,
    PRE_DELIVERING,
    DELIVERING,
    DELIVERED,
    PRE_SETTLE,
    SETTLING,
    CLOSE
};

enum class IncomeType : std::int32_t {
    TRANSFER,
    WELCOME_BONUS,
    REALIZED_PNL,
    FUNDING_FEE,
    COMMISSION,
    INSURANCE_CLEAR,
    REFERRAL_KICKBACK,
    COMMISSION_REBATE,
    DELIVERED_SETTLEMENT,
    COIN_SWAP_DEPOSIT,
    COIN_SWAP_WITHDRAW,
    ALL
};

enum class MarginType : std::int32_t {
    ISOLATED,
    CROSS
};

enum class StatisticsPeriod : std::int32_t {
    _5m,
    _15m,
    _30m,
    _1h,
    _2h,
    _4h,
    _6h,
    _12h,
    _1d
};

enum class SelfTradePreventionMode : std::int32_t {
    NONE,
    EXPIRE_TAKER,
    EXPIRE_BOTH,
    EXPIRE_MAKER
};

struct ErrorResponse final : IJson {
    int m_code{};

    std::string m_msg{};

    [[nodiscard]] nlohmann::json toJson() const override;

    void fromJson(const nlohmann::json &json) override;
};

struct FundingRate final : IJson {
    std::string m_symbol{};
    double m_fundingRate{};
    std::int64_t m_fundingTime{};

    [[nodiscard]] nlohmann::json toJson() const override;

    void fromJson(const nlohmann::json &json) override;
};

struct FundingRates final : IJson {
    std::vector<FundingRate> m_fundingRates{};

    [[nodiscard]] nlohmann::json toJson() const override;

    void fromJson(const nlohmann::json &json) override;
};

struct TickerPrice final : IJson {
    std::string m_symbol{};
    double m_price{};
    std::int64_t m_time{};

    [[nodiscard]] nlohmann::json toJson() const override;

    void fromJson(const nlohmann::json &json) override;
};

struct BookTickerPrice final : IJson {
    std::string m_symbol{};
    double m_bidPrice{};
    double m_askPrice{};
    double m_bidQty{};
    double m_askQty{};
    std::int64_t m_time{};

    [[nodiscard]] nlohmann::json toJson() const override;

    void fromJson(const nlohmann::json &json) override;
};

struct MarkPrice final : IJson {
    std::string m_symbol{};
    double m_markPrice{};
    double m_indexPrice{};
    double m_estimatedSettlePrice{};
    double m_lastFundingRate{};
    std::int64_t m_nextFundingTime{};
    double m_interestRate{};
    std::int64_t m_time{};

    [[nodiscard]] nlohmann::json toJson() const override;

    void fromJson(const nlohmann::json &json) override;
};

struct MarkPrices final : IJson {
    std::vector<MarkPrice> m_markPrices{};

    [[nodiscard]] nlohmann::json toJson() const override;

    void fromJson(const nlohmann::json &json) override;
};

struct Asset final : IJson {
    std::string m_asset{};
    double m_walletBalance{};
    double m_unrealizedProfit{};
    double m_marginBalance{};
    double m_maintMargin{};
    double m_initialMargin{};
    double m_positionInitialMargin{};
    double m_openOrderInitialMargin{};
    double m_crossWalletBalance{};
    double m_crossUnPnl{};
    double m_availableBalance{};
    double m_maxWithdrawAmount{};
    bool m_marginAvailable{false};
    std::int64_t m_updateTime{};
    double m_autoAssetExchange{};

    [[nodiscard]] nlohmann::json toJson() const override;

    void fromJson(const nlohmann::json &json) override;
};

struct Account final : IJson {
    int m_feeTier{};
    bool m_canTrade{false};
    bool m_canDeposit{false};
    bool m_canWithdraw{false};
    std::int64_t m_updateTime{};
    double m_totalInitialMargin{};
    double m_totalMaintMargin{};
    double m_totalWalletBalance{};
    double m_totalUnrealizedProfit{};
    double m_totalMarginBalance{};
    double m_totalPositionInitialMargin{};
    double m_totalOpenOrderInitialMargin{};
    double m_totalCrossWalletBalance{};
    double m_totalCrossUnPnl{};
    double m_availableBalance{};
    double m_maxWithdrawAmount{};
    int m_tradeGroupId = -1;

    std::vector<Asset> m_assets;

    [[nodiscard]] nlohmann::json toJson() const override;

    void fromJson(const nlohmann::json &json) override;
};

struct AccountBalance final : IJson {
    std::string m_accountAlias{};
    std::string m_asset{};
    double m_balance{};
    double m_crossWalletBalance{};
    double m_crossUnPnl{};
    double m_availableBalance{};
    double m_maxWithdrawAmount{};
    bool m_marginAvailable{true};
    std::int64_t m_updateTime{};

    [[nodiscard]] nlohmann::json toJson() const override;

    void fromJson(const nlohmann::json &json) override;
};

struct Order : IJson {
    std::string m_symbol{};
    Side m_side{Side::BUY};
    PositionSide m_positionSide{PositionSide::BOTH};
    OrderType m_type{OrderType::LIMIT};
    TimeInForce m_timeInForce{TimeInForce::GTC};
    std::int64_t m_orderId{};

    /// Cannot be sent with closePosition=true(Close-All)
    double m_quantity{};

    /// "true" or "false". default "false". Cannot be sent in Hedge Mode; cannot be sent with closePosition=true
    bool m_reduceOnly{false};

    /// A unique id among open orders. Automatically generated if not sent. Can only be string following
    /// the rule: ^[\.A-Z\:/a-z0-9_-]{1,36}$
    std::string m_newClientOrderId{};

    double m_price{};

    /// Used with STOP/STOP_MARKET or TAKE_PROFIT/TAKE_PROFIT_MARKET orders.
    double m_stopPrice{};

    int64_t m_timestamp{};

    /// true, false；Close-All，used with STOP_MARKET or TAKE_PROFIT_MARKET.
    bool m_closePosition{false};

    /// Used with TRAILING_STOP_MARKET orders, default as the latest price(supporting different workingType)
    double m_activationPrice{};

    /// Used with TRAILING_STOP_MARKET orders, min 0.1, max 5 where 1 for 1%
    double m_callbackRate{};

    /// stopPrice triggered by: "MARK_PRICE", "CONTRACT_PRICE". Default "CONTRACT_PRICE"
    WorkingType m_workingType{WorkingType::CONTRACT_PRICE};

    /// "TRUE" or "FALSE", default "FALSE". Used with STOP/STOP_MARKET or TAKE_PROFIT/TAKE_PROFIT_MARKET orders.
    bool m_priceProtect{false};

    OrderRespType m_newOrderRespType{OrderRespType::ACK};

    /// quantityPrecision is not part of Binance API, it serves for formatting only (shitty API design!)
    int m_quantityPrecision{2};

    /// pricePrecision is not part of Binance API, it serves for formatting only (shitty API design!)
    int m_pricePrecision{2};

    SelfTradePreventionMode m_selfTradePreventionMode{SelfTradePreventionMode::NONE};

    [[nodiscard]] nlohmann::json toJson() const override;

    void fromJson(const nlohmann::json &json) override;
};

struct OrderResponse final : Order {
    std::int64_t m_orderId{};
    std::string m_clientOrderId{};
    OrderStatus m_orderStatus{OrderStatus::NEW};
    double m_avgPrice{};
    double m_origQty{};
    double m_executedQty{};
    double m_cumQty{};
    double m_cumQuote{};
    OrderType m_origType{OrderType::LIMIT};
    int m_errCode{};
    std::string m_errMsg{};

    [[nodiscard]] nlohmann::json toJson() const override;

    void fromJson(const nlohmann::json &json) override;
};

struct OrdersResponse final : IJson {
    std::vector<OrderResponse> m_responses{};

    [[nodiscard]] nlohmann::json toJson() const override;

    void fromJson(const nlohmann::json &json) override;
};

struct Candle final : IJson {
    std::int64_t m_openTime{};
    double m_open{};
    double m_high{};
    double m_low{};
    double m_close{};
    double m_volume{};
    std::int64_t m_closeTime{};
    double m_quoteVolume{};
    std::int64_t m_numberOfTrades{};
    double m_takerBuyVolume{};
    double m_takerQuoteVolume{};
    std::string m_ignore{};

    [[nodiscard]] nlohmann::json toJson() const override;

    void fromJson(const nlohmann::json &json) override;
};

struct CandlesResponse final : IJson {
    std::vector<Candle> m_candles;

    [[nodiscard]] nlohmann::json toJson() const override;

    void fromJson(const nlohmann::json &json) override;
};

struct Position final : IJson {
    double m_entryPrice{};
    std::string m_marginType{};
    bool m_isAutoAddMargin{false};
    double m_isolatedMargin{};
    double m_leverage{};
    double m_liquidationPrice{};
    double m_markPrice{};
    double m_maxNotionalValue{};
    double m_positionAmt{};
    std::string m_symbol{};
    double m_unRealizedProfit{};
    PositionSide m_positionSide{PositionSide::BOTH};
    std::int64_t m_updateTime{};

    [[nodiscard]] nlohmann::json toJson() const override;

    void fromJson(const nlohmann::json &json) override;
};

struct Filter final : IJson {
    SymbolFilter m_filterType{SymbolFilter::PRICE_FILTER};
    double m_maxPrice{};
    double m_minPrice{};
    double m_tickSize{};
    double m_minQty{};
    double m_maxQty{};
    double m_stepSize{};
    std::int64_t m_limit{};
    double m_multiplierUp{};
    double m_multiplierDown{};
    double m_multiplierDecimal{};
    double m_notional{};

    [[nodiscard]] nlohmann::json toJson() const override;

    void fromJson(const nlohmann::json &json) override;
};

struct Symbol final : IJson {
    std::string m_symbol{};
    std::string m_pair{};
    std::string m_contractType{};
    std::int64_t m_deliveryDate{};
    std::int64_t m_onboardDate{};
    ContractStatus m_status{ContractStatus::TRADING};
    double m_maintMarginPercent{};
    double m_requiredMarginPercent{};
    std::string m_baseAsset{};
    std::string m_quoteAsset{};
    std::string m_marginAsset{};
    int m_pricePrecision{};
    int m_quantityPrecision{};
    int m_baseAssetPrecision{};
    int m_quotePrecision{};
    std::string m_underlyingType{};
    std::vector<std::string> m_underlyingSubType{};
    std::int64_t m_settlePlan{};
    double m_triggerProtect{};
    std::vector<Filter> m_filters{};
    std::vector<OrderType> m_orderType{};
    std::vector<TimeInForce> m_timeInForce{};
    double m_liquidationFee{};
    double m_marketTakeBound{};

    [[nodiscard]] nlohmann::json toJson() const override;

    void fromJson(const nlohmann::json &json) override;
};

struct RateLimit final : IJson {
    RateLimitInterval m_interval{RateLimitInterval::MINUTE};
    std::int32_t m_intervalNum{};
    std::int32_t m_limit{};
    RateLimitType m_rateLimitType{RateLimitType::REQUEST_WEIGHT};

    [[nodiscard]] nlohmann::json toJson() const override;

    void fromJson(const nlohmann::json &json) override;
};

struct Exchange final : IJson {
    std::vector<RateLimit> m_rateLimits{};
    std::vector<Asset> m_assets{};
    std::vector<Symbol> m_symbols{};
    std::int64_t m_serverTime{};
    std::string m_timezone{};

    /// lastUpdateTime is not part of Binance API, it serves for keeping Exchange data up to date
    std::int64_t m_lastUpdateTime{-1};

    [[nodiscard]] nlohmann::json toJson() const override;

    void fromJson(const nlohmann::json &json) override;
};

struct DownloadId final : IJson {
    std::int64_t m_avgCostTimestampOfLast30d{};
    std::string m_downloadId{};

    [[nodiscard]] nlohmann::json toJson() const override;

    void fromJson(const nlohmann::json &json) override;
};

struct Income final : IJson {
    std::string m_symbol{};
    IncomeType m_incomeType{IncomeType::TRANSFER};
    double m_income{};
    std::string m_asset{};
    std::string m_info{};
    std::int64_t m_time{};
    std::int64_t m_tranId{};
    std::string m_tradeId{};

    [[nodiscard]] nlohmann::json toJson() const override;

    void fromJson(const nlohmann::json &json) override;
};

struct Incomes final : IJson {
    std::vector<Income> m_incomes{};

    [[nodiscard]] nlohmann::json toJson() const override;

    void fromJson(const nlohmann::json &json) override;
};

struct PositionRisk final : IJson {
    std::string m_symbol{};
    double m_entryPrice{};
    MarginType m_marginType{MarginType::ISOLATED};
    bool m_isAutoAddMargin{false};
    double m_isolatedMargin{};
    int m_leverage{1};
    double m_liquidationPrice{};
    double m_markPrice{};
    double m_maxNotionalValue{};
    double m_positionAmt{};
    double m_notional{};
    double m_isolatedWallet{};
    double m_unRealizedProfit{};
    PositionSide m_positionSide{PositionSide::BOTH};
    std::int64_t m_updateTime{};

    [[nodiscard]] nlohmann::json toJson() const override;

    void fromJson(const nlohmann::json &json) override;
};

struct OpenInterest final : IJson {
    std::string m_symbol{};
    double m_openInterest{};
    std::int64_t m_time{};

    [[nodiscard]] nlohmann::json toJson() const override;

    void fromJson(const nlohmann::json &json) override;
};

struct LongShortRatio final : IJson {
    std::string m_symbol{};
    double m_longShortRatio{};
    double m_longAccount{};
    double m_shortAccount{};
    std::int64_t m_timestamp{};

    [[nodiscard]] nlohmann::json toJson() const override;

    void fromJson(const nlohmann::json &json) override;
};

struct OpenInterestStatistics final : IJson {
    std::string m_symbol{};
    double m_sumOpenInterest{};
    double m_sumOpenInterestValue{};
    std::int64_t m_timestamp{};

    [[nodiscard]] nlohmann::json toJson() const override;

    void fromJson(const nlohmann::json &json) override;
};

struct BuySellVolume final : IJson {
    double m_buySellRatio{};
    double m_buyVol{};
    double m_sellVol{};
    std::int64_t m_timestamp{};

    [[nodiscard]] nlohmann::json toJson() const override;

    void fromJson(const nlohmann::json &json) override;
};
}
#endif //INCLUDE_CK_BINANCE_MODELS_H
