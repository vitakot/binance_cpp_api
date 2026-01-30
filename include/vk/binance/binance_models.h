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
 * GTC - Good till canceled
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

struct RateLimit final : IJson {
    RateLimitInterval interval{RateLimitInterval::MINUTE};
    std::int32_t intervalNum{};
    std::int32_t limit{};
    RateLimitType rateLimitType{RateLimitType::REQUEST_WEIGHT};

    [[nodiscard]] nlohmann::json toJson() const override;

    void fromJson(const nlohmann::json &json) override;
};

struct ErrorResponse final : IJson {
    int code{};
    std::string msg{};

    [[nodiscard]] nlohmann::json toJson() const override;

    void fromJson(const nlohmann::json &json) override;
};

struct Candle final : IJson {
    std::int64_t openTime{};
    double open{};
    double high{};
    double low{};
    double close{};
    double volume{};
    std::int64_t closeTime{};
    double quoteVolume{};
    std::int64_t numberOfTrades{};
    double takerBuyVolume{};
    double takerQuoteVolume{};
    std::string ignore{};

    [[nodiscard]] nlohmann::json toJson() const override;

    void fromJson(const nlohmann::json &json) override;
};

struct CandlesResponse final : IJson {
    std::vector<Candle> candles;

    [[nodiscard]] nlohmann::json toJson() const override;

    void fromJson(const nlohmann::json &json) override;
};
}

namespace vk::binance::spot {

struct Symbol final : IJson {
    std::string symbol{};
    ContractStatus status{ContractStatus::TRADING};
    std::string baseAsset{};
    std::string quoteAsset{};

    [[nodiscard]] nlohmann::json toJson() const override;

    void fromJson(const nlohmann::json &json) override;
};

struct Exchange final : IJson {
    std::vector<RateLimit> rateLimits{};
    std::vector<Symbol> symbols{};

    /// lastUpdateTime is not part of Binance API, it serves for keeping Exchange data up to date
    std::int64_t lastUpdateTime{-1};

    [[nodiscard]] nlohmann::json toJson() const override;

    void fromJson(const nlohmann::json &json) override;
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

enum class SymbolFilter : std::int32_t {
    PRICE_FILTER,
    LOT_SIZE,
    MARKET_LOT_SIZE,
    MAX_NUM_ORDERS,
    MAX_NUM_ALGO_ORDERS,
    PERCENT_PRICE,
    MIN_NOTIONAL,
    POSITION_RISK_CONTROL
};

enum class ContractType : std::int32_t {
    PERPETUAL,
    CURRENT_MONTH,
    NEXT_MONTH,
    CURRENT_QUARTER,
    NEXT_QUARTER
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

struct FundingRate final : IJson {
    std::string symbol{};
    double fundingRate{};
    std::int64_t fundingTime{};

    [[nodiscard]] nlohmann::json toJson() const override;

    void fromJson(const nlohmann::json &json) override;
};

struct FundingRates final : IJson {
    std::vector<FundingRate> fundingRates{};

    [[nodiscard]] nlohmann::json toJson() const override;

    void fromJson(const nlohmann::json &json) override;
};

struct TickerPrice final : IJson {
    std::string symbol{};
    double price{};
    std::int64_t time{};

    [[nodiscard]] nlohmann::json toJson() const override;

    void fromJson(const nlohmann::json &json) override;
};

struct BookTickerPrice final : IJson {
    std::string symbol{};
    double bidPrice{};
    double askPrice{};
    double bidQty{};
    double askQty{};
    std::int64_t time{};

    [[nodiscard]] nlohmann::json toJson() const override;

    void fromJson(const nlohmann::json &json) override;
};

struct MarkPrice final : IJson {
    std::string symbol{};
    double markPrice{};
    double indexPrice{};
    double estimatedSettlePrice{};
    double lastFundingRate{};
    std::int64_t nextFundingTime{};
    double interestRate{};
    std::int64_t time{};

    [[nodiscard]] nlohmann::json toJson() const override;

    void fromJson(const nlohmann::json &json) override;
};

struct MarkPrices final : IJson {
    std::vector<MarkPrice> markPrices{};

    [[nodiscard]] nlohmann::json toJson() const override;

    void fromJson(const nlohmann::json &json) override;
};

struct Asset final : IJson {
    std::string asset{};
    double walletBalance{};
    double unrealizedProfit{};
    double marginBalance{};
    double maintMargin{};
    double initialMargin{};
    double positionInitialMargin{};
    double openOrderInitialMargin{};
    double crossWalletBalance{};
    double crossUnPnl{};
    double availableBalance{};
    double maxWithdrawAmount{};
    bool marginAvailable{false};
    std::int64_t updateTime{};
    double autoAssetExchange{};

    [[nodiscard]] nlohmann::json toJson() const override;

    void fromJson(const nlohmann::json &json) override;
};

struct Account final : IJson {
    int feeTier{};
    bool canTrade{false};
    bool canDeposit{false};
    bool canWithdraw{false};
    std::int64_t updateTime{};
    double totalInitialMargin{};
    double totalMaintMargin{};
    double totalWalletBalance{};
    double totalUnrealizedProfit{};
    double totalMarginBalance{};
    double totalPositionInitialMargin{};
    double totalOpenOrderInitialMargin{};
    double totalCrossWalletBalance{};
    double totalCrossUnPnl{};
    double availableBalance{};
    double maxWithdrawAmount{};
    int tradeGroupId = -1;

    std::vector<Asset> assets;

    [[nodiscard]] nlohmann::json toJson() const override;

    void fromJson(const nlohmann::json &json) override;
};

struct AccountBalance final : IJson {
    std::string accountAlias{};
    std::string asset{};
    double balance{};
    double crossWalletBalance{};
    double crossUnPnl{};
    double availableBalance{};
    double maxWithdrawAmount{};
    bool marginAvailable{true};
    std::int64_t updateTime{};

    [[nodiscard]] nlohmann::json toJson() const override;

    void fromJson(const nlohmann::json &json) override;
};

struct Order : IJson {
    std::string symbol{};
    Side side{Side::BUY};
    PositionSide positionSide{PositionSide::BOTH};
    OrderType type{OrderType::LIMIT};
    TimeInForce timeInForce{TimeInForce::GTC};
    std::int64_t orderId{};

    /// Cannot be sent with closePosition=true(Close-All)
    double quantity{};

    /// "true" or "false". default "false". Cannot be sent in Hedge Mode; cannot be sent with closePosition=true
    bool reduceOnly{false};

    /// A unique id among open orders. Automatically generated if not sent. Can only be string following
    /// the rule: ^[\.A-Z\:/a-z0-9_-]{1,36}$
    std::string newClientOrderId{};

    double price{};

    /// Used with STOP/STOP_MARKET or TAKE_PROFIT/TAKE_PROFIT_MARKET orders.
    double stopPrice{};

    int64_t timestamp{};

    /// true, false；Close-All，used with STOP_MARKET or TAKE_PROFIT_MARKET.
    bool closePosition{false};

    /// Used with TRAILING_STOP_MARKET orders, default as the latest price(supporting different workingType)
    double activationPrice{};

    /// Used with TRAILING_STOP_MARKET orders, min 0.1, max 5 where 1 for 1%
    double callbackRate{};

    /// stopPrice triggered by: "MARK_PRICE", "CONTRACT_PRICE". Default "CONTRACT_PRICE"
    WorkingType workingType{WorkingType::CONTRACT_PRICE};

    /// "TRUE" or "FALSE", default "FALSE". Used with STOP/STOP_MARKET or TAKE_PROFIT/TAKE_PROFIT_MARKET orders.
    bool priceProtect{false};

    OrderRespType newOrderRespType{OrderRespType::ACK};

    /// quantityPrecision is not part of Binance API, it serves for formatting only (shitty API design!)
    int quantityPrecision{2};

    /// pricePrecision is not part of Binance API, it serves for formatting only (shitty API design!)
    int pricePrecision{2};

    SelfTradePreventionMode selfTradePreventionMode{SelfTradePreventionMode::NONE};

    [[nodiscard]] nlohmann::json toJson() const override;

    void fromJson(const nlohmann::json &json) override;
};

struct OrderResponse final : Order {
    std::int64_t orderId{};
    std::string clientOrderId{};
    OrderStatus orderStatus{OrderStatus::NEW};
    double avgPrice{};
    double origQty{};
    double executedQty{};
    double cumQty{};
    double cumQuote{};
    OrderType origType{OrderType::LIMIT};
    int errCode{};
    std::string errMsg{};

    [[nodiscard]] nlohmann::json toJson() const override;

    void fromJson(const nlohmann::json &json) override;
};

struct OrdersResponse final : IJson {
    std::vector<OrderResponse> responses{};

    [[nodiscard]] nlohmann::json toJson() const override;

    void fromJson(const nlohmann::json &json) override;
};

struct Position final : IJson {
    double entryPrice{};
    std::string marginType{};
    bool isAutoAddMargin{false};
    double isolatedMargin{};
    double leverage{};
    double liquidationPrice{};
    double markPrice{};
    double maxNotionalValue{};
    double positionAmt{};
    std::string symbol{};
    double unRealizedProfit{};
    PositionSide positionSide{PositionSide::BOTH};
    std::int64_t updateTime{};

    [[nodiscard]] nlohmann::json toJson() const override;

    void fromJson(const nlohmann::json &json) override;
};

struct Filter final : IJson {
    SymbolFilter filterType{SymbolFilter::PRICE_FILTER};
    double maxPrice{};
    double minPrice{};
    double tickSize{};
    double minQty{};
    double maxQty{};
    double stepSize{};
    std::int64_t limit{};
    double multiplierUp{};
    double multiplierDown{};
    double multiplierDecimal{};
    double notional{};

    [[nodiscard]] nlohmann::json toJson() const override;

    void fromJson(const nlohmann::json &json) override;
};

struct Symbol final : IJson {
    std::string symbol{};
    std::string pair{};
    std::string contractType{};
    std::int64_t deliveryDate{};
    std::int64_t onboardDate{};
    ContractStatus status{ContractStatus::TRADING};
    double maintMarginPercent{};
    double requiredMarginPercent{};
    std::string baseAsset{};
    std::string quoteAsset{};
    std::string marginAsset{};
    int pricePrecision{};
    int quantityPrecision{};
    int baseAssetPrecision{};
    int quotePrecision{};
    std::string underlyingType{};
    std::vector<std::string> underlyingSubType{};
    std::int64_t settlePlan{};
    double triggerProtect{};
    std::vector<Filter> filters{};
    std::vector<OrderType> orderType{};
    std::vector<TimeInForce> timeInForce{};
    double liquidationFee{};
    double marketTakeBound{};

    [[nodiscard]] nlohmann::json toJson() const override;

    void fromJson(const nlohmann::json &json) override;
};

struct Exchange final : IJson {
    std::vector<RateLimit> rateLimits{};
    std::vector<Asset> assets{};
    std::vector<Symbol> symbols{};
    std::int64_t serverTime{};
    std::string timezone{};

    /// lastUpdateTime is not part of Binance API, it serves for keeping Exchange data up to date
    std::int64_t lastUpdateTime{-1};

    [[nodiscard]] nlohmann::json toJson() const override;

    void fromJson(const nlohmann::json &json) override;
};

struct DownloadId final : IJson {
    std::int64_t avgCostTimestampOfLast30d{};
    std::string downloadId{};

    [[nodiscard]] nlohmann::json toJson() const override;

    void fromJson(const nlohmann::json &json) override;
};

struct Income final : IJson {
    std::string symbol{};
    IncomeType incomeType{IncomeType::TRANSFER};
    double income{};
    std::string asset{};
    std::string info{};
    std::int64_t time{};
    std::int64_t tranId{};
    std::string tradeId{};

    [[nodiscard]] nlohmann::json toJson() const override;

    void fromJson(const nlohmann::json &json) override;
};

struct Incomes final : IJson {
    std::vector<Income> incomes{};

    [[nodiscard]] nlohmann::json toJson() const override;

    void fromJson(const nlohmann::json &json) override;
};

struct PositionRisk final : IJson {
    std::string symbol{};
    double entryPrice{};
    MarginType marginType{MarginType::ISOLATED};
    bool isAutoAddMargin{false};
    double isolatedMargin{};
    int leverage{1};
    double liquidationPrice{};
    double markPrice{};
    double maxNotionalValue{};
    double positionAmt{};
    double notional{};
    double isolatedWallet{};
    double unRealizedProfit{};
    PositionSide positionSide{PositionSide::BOTH};
    std::int64_t updateTime{};

    [[nodiscard]] nlohmann::json toJson() const override;

    void fromJson(const nlohmann::json &json) override;
};

struct OpenInterest final : IJson {
    std::string symbol{};
    double openInterest{};
    std::int64_t time{};

    [[nodiscard]] nlohmann::json toJson() const override;

    void fromJson(const nlohmann::json &json) override;
};

struct LongShortRatio final : IJson {
    std::string symbol{};
    double longShortRatio{};
    double longAccount{};
    double shortAccount{};
    std::int64_t timestamp{};

    [[nodiscard]] nlohmann::json toJson() const override;

    void fromJson(const nlohmann::json &json) override;
};

struct OpenInterestStatistics final : IJson {
    std::string symbol{};
    double sumOpenInterest{};
    double sumOpenInterestValue{};
    std::int64_t timestamp{};

    [[nodiscard]] nlohmann::json toJson() const override;

    void fromJson(const nlohmann::json &json) override;
};

struct BuySellVolume final : IJson {
    double buySellRatio{};
    double buyVol{};
    double sellVol{};
    std::int64_t timestamp{};

    [[nodiscard]] nlohmann::json toJson() const override;

    void fromJson(const nlohmann::json &json) override;
};
}
#endif //INCLUDE_CK_BINANCE_MODELS_H
