/**
Binance Data Models

Licensed under the MIT License <http://opensource.org/licenses/MIT>.
SPDX-License-Identifier: MIT
Copyright (c) 2022 Vitezslav Kot <vitezslav.kot@gmail.com>.
*/

#include "vk/binance/binance_models.h"
#include "vk/tools//utils.h"
#include "vk/tools/json_utils.h"

namespace vk::binance::futures {
nlohmann::json ErrorResponse::toJson() const {
    throw std::runtime_error("Unimplemented: ErrorResponse::toJson()");
}

void ErrorResponse::fromJson(const nlohmann::json &json) {
    readValue<int>(json, "code", m_code);
    readValue<std::string>(json, "msg", m_msg);
}

nlohmann::json FundingRate::toJson() const {
    throw std::runtime_error("Unimplemented: FundingRate::toJson()");
}

void FundingRate::fromJson(const nlohmann::json &json) {
    readValue<std::string>(json, "symbol", m_symbol);
    readValue<std::int64_t>(json, "fundingTime", m_fundingTime);
    m_fundingRate = readStringAsDouble(json, "fundingRate");
}

nlohmann::json FundingRates::toJson() const {
    throw std::runtime_error("Unimplemented: FundingRates::toJson()");
}

void FundingRates::fromJson(const nlohmann::json &json) {
    m_fundingRates.clear();

    for (const auto &el: json) {
        FundingRate fundingRate;
        fundingRate.fromJson(el);
        m_fundingRates.push_back(fundingRate);
    }
}

nlohmann::json TickerPrice::toJson() const {
    throw std::runtime_error("Unimplemented: TickerPrice::toJson()");
}

void TickerPrice::fromJson(const nlohmann::json &json) {
    readValue<std::string>(json, "symbol", m_symbol);
    m_price = readStringAsDouble(json, "price");
    readValue<std::int64_t>(json, "time", m_time);
}

nlohmann::json BookTickerPrice::toJson() const {
    throw std::runtime_error("Unimplemented: BookTickerPrice::toJson()");
}

void BookTickerPrice::fromJson(const nlohmann::json &json) {
    readValue<std::string>(json, "symbol", m_symbol);
    m_bidPrice = readStringAsDouble(json, "bidPrice");
    m_askPrice = readStringAsDouble(json, "askPrice");
    m_bidQty = readStringAsDouble(json, "bidQty");
    m_askQty = readStringAsDouble(json, "askQty");
    readValue<std::int64_t>(json, "time", m_time);
}

nlohmann::json MarkPrice::toJson() const {
    throw std::runtime_error("Unimplemented: MarkPrice::toJson()");
}

void MarkPrice::fromJson(const nlohmann::json &json) {
    readValue<std::string>(json, "symbol", m_symbol);
    readValue<std::int64_t>(json, "nextFundingTime", m_nextFundingTime);
    readValue<std::int64_t>(json, "time", m_time);
    m_markPrice = readStringAsDouble(json, "markPrice");
    m_indexPrice = readStringAsDouble(json, "indexPrice");
    m_estimatedSettlePrice = readStringAsDouble(json, "estimatedSettlePrice");
    m_lastFundingRate = readStringAsDouble(json, "lastFundingRate");
    m_interestRate = readStringAsDouble(json, "interestRate");
}

nlohmann::json MarkPrices::toJson() const {
    throw std::runtime_error("Unimplemented: MarkPrices::toJson()");
}

void MarkPrices::fromJson(const nlohmann::json &json) {
    m_markPrices.clear();

    for (const auto &el: json) {
        MarkPrice markPrice;
        markPrice.fromJson(el);
        m_markPrices.push_back(markPrice);
    }
}

nlohmann::json Asset::toJson() const {
    throw std::runtime_error("Unimplemented: Asset::toJson()");
}

void Asset::fromJson(const nlohmann::json &json) {
    readValue<std::string>(json, "asset", m_asset);
    m_walletBalance = readStringAsDouble(json, "walletBalance");
    m_unrealizedProfit = readStringAsDouble(json, "unrealizedProfit");
    m_marginBalance = readStringAsDouble(json, "marginBalance");
    m_maintMargin = readStringAsDouble(json, "maintMargin");
    m_initialMargin = readStringAsDouble(json, "initialMargin");
    m_positionInitialMargin = readStringAsDouble(json, "positionInitialMargin");
    m_openOrderInitialMargin = readStringAsDouble(json, "openOrderInitialMargin");
    m_crossWalletBalance = readStringAsDouble(json, "crossWalletBalance");
    m_crossUnPnl = readStringAsDouble(json, "crossUnPnl");
    m_availableBalance = readStringAsDouble(json, "availableBalance");
    m_maxWithdrawAmount = readStringAsDouble(json, "maxWithdrawAmount");
    readValue<bool>(json, "marginAvailable", m_marginAvailable);
    readValue<int64_t>(json, "updateTime", m_updateTime);
    m_autoAssetExchange = readStringAsDouble(json, "autoAssetExchange");
}

nlohmann::json Account::toJson() const {
    throw std::runtime_error("Unimplemented: Account::toJson()");
}

void Account::fromJson(const nlohmann::json &json) {
    readValue<int>(json, "feeTier", m_feeTier);
    readValue<bool>(json, "canTrade", m_canTrade);
    readValue<bool>(json, "canDeposit", m_canDeposit);
    readValue<bool>(json, "canWithdraw", m_canWithdraw);
    readValue<int64_t>(json, "updateTime", m_updateTime);
    m_totalInitialMargin = readStringAsDouble(json, "totalInitialMargin");
    m_totalMaintMargin = readStringAsDouble(json, "totalMaintMargin");
    m_totalWalletBalance = readStringAsDouble(json, "totalWalletBalance");
    m_totalUnrealizedProfit = readStringAsDouble(json, "totalUnrealizedProfit");
    m_totalMarginBalance = readStringAsDouble(json, "totalMarginBalance");
    m_totalPositionInitialMargin = readStringAsDouble(json, "totalPositionInitialMargin");
    m_totalOpenOrderInitialMargin = readStringAsDouble(json, "totalOpenOrderInitialMargin");
    m_totalCrossWalletBalance = readStringAsDouble(json, "totalCrossWalletBalance");
    m_totalCrossUnPnl = readStringAsDouble(json, "totalCrossUnPnl");
    m_availableBalance = readStringAsDouble(json, "availableBalance");
    m_maxWithdrawAmount = readStringAsDouble(json, "maxWithdrawAmount");
    readValue<int>(json, "tradeGroupId", m_tradeGroupId);

    m_assets.clear();

    for (const auto &el: json["assets"]) {
        Asset asset;
        asset.fromJson(el);
        m_assets.push_back(asset);
    }
}

nlohmann::json AccountBalance::toJson() const {
    throw std::runtime_error("Unimplemented: AccountBalance::toJson()");
}

void AccountBalance::fromJson(const nlohmann::json &json) {
    readValue<std::string>(json, "accountAlias", m_accountAlias);
    readValue<std::string>(json, "asset", m_asset);
    m_balance = readStringAsDouble(json, "balance");
    m_crossWalletBalance = readStringAsDouble(json, "crossWalletBalance");
    m_crossUnPnl = readStringAsDouble(json, "crossUnPnl");
    m_availableBalance = readStringAsDouble(json, "availableBalance");
    m_maxWithdrawAmount = readStringAsDouble(json, "maxWithdrawAmount");
    readValue<bool>(json, "marginAvailable", m_marginAvailable);
    readValue<std::int64_t>(json, "updateTime", m_updateTime);
}

nlohmann::json Order::toJson() const {
    nlohmann::json json;
    json["symbol"] = m_symbol;
    json["side"] = magic_enum::enum_name(m_side);
    json["positionSide"] = magic_enum::enum_name(m_positionSide);
    json["type"] = magic_enum::enum_name(m_type);
    json["orderId"] = std::to_string(m_orderId);

    if (!m_newClientOrderId.empty()) {
        json["newClientOrderId"] = m_newClientOrderId;
    }

    json["newOrderRespType"] = magic_enum::enum_name(m_newOrderRespType);

    if (!m_closePosition) {
        json["reduceOnly"] = fmt::format("{}", m_reduceOnly);
    }

    if (m_type == OrderType::LIMIT) {
        json["timeInForce"] = magic_enum::enum_name(m_timeInForce);
        json["quantity"] = formatDouble(m_quantityPrecision, m_quantity);
        json["price"] = formatDouble(m_pricePrecision, m_price);
    } else if (m_type == OrderType::MARKET) {
        json["quantity"] = formatDouble(m_quantityPrecision, m_quantity);
    } else if (m_type == OrderType::STOP ||
               m_type == OrderType::TAKE_PROFIT) {
        json["quantity"] = formatDouble(m_quantityPrecision, m_quantity);
        json["price"] = formatDouble(m_pricePrecision, m_price);
        json["stopPrice"] = formatDouble(m_pricePrecision, m_stopPrice);
    } else if (m_type == OrderType::STOP_MARKET ||
               m_type == OrderType::TAKE_PROFIT_MARKET) {
        json["quantity"] = formatDouble(m_quantityPrecision, m_quantity);
        json["stopPrice"] = formatDouble(m_pricePrecision, m_stopPrice);
        json["priceProtect"] = fmt::format("{}", m_priceProtect);
        json["closePosition"] = fmt::format("{}", m_closePosition);
    } else if (m_type == OrderType::TRAILING_STOP_MARKET) {
        json["quantity"] = formatDouble(m_quantityPrecision, m_quantity);
        json["callbackRate"] = std::to_string(m_callbackRate);
        json["activationPrice"] = formatDouble(m_pricePrecision, m_activationPrice);
    }

    json["selfTradePreventionMode"] = magic_enum::enum_name(m_selfTradePreventionMode);

    return json;
}

void Order::fromJson(const nlohmann::json &json) {
    readValue<std::string>(json, "symbol", m_symbol);
    readMagicEnum<Side>(json, "side", m_side);
    m_price = readStringAsDouble(json, "price");
    readMagicEnum<PositionSide>(json, "positionSide", m_positionSide);
    readMagicEnum<OrderType>(json, "type", m_type);
    readMagicEnum<TimeInForce>(json, "timeInForce", m_timeInForce);
    m_quantity = readStringAsDouble(json, "quantity");
    readValue<std::string>(json, "newClientOrderId", m_newClientOrderId);
    m_stopPrice = readStringAsDouble(json, "stopPrice");
    readValue<int64_t>(json, "timestamp", m_timestamp);
    readValue<int64_t>(json, "orderId", m_orderId);
    readValue<bool>(json, "reduceOnly", m_reduceOnly);
    readValue<bool>(json, "closePosition", m_closePosition);
    m_activationPrice = readStringAsDouble(json, "activationPrice");
    m_callbackRate = readStringAsDouble(json, "callbackRate");
    readMagicEnum<WorkingType>(json, "workingType", m_workingType);
    readValue<bool>(json, "priceProtect", m_priceProtect);
    readMagicEnum<OrderRespType>(json, "newOrderRespType", m_newOrderRespType);
    readMagicEnum<SelfTradePreventionMode>(json, "selfTradePreventionMode", m_selfTradePreventionMode);
}

nlohmann::json OrderResponse::toJson() const {
    nlohmann::json json = Order::toJson();

    json["orderId"] = m_orderId;
    json["clientOrderId"] = m_clientOrderId;
    json["status"] = magic_enum::enum_name(m_orderStatus);
    json["avgPrice"] = std::to_string(m_avgPrice);
    json["origQty"] = std::to_string(m_origQty);
    json["executedQty"] = std::to_string(m_executedQty);
    json["cumQty"] = std::to_string(m_cumQty);
    json["cumQuote"] = std::to_string(m_cumQuote);
    json["origType"] = magic_enum::enum_name(m_origType);
    return json;
}

void OrderResponse::fromJson(const nlohmann::json &json) {
    Order::fromJson(json);

    readValue<int64_t>(json, "orderId", m_orderId);
    readValue<std::string>(json, "clientOrderId", m_clientOrderId);
    readMagicEnum<OrderStatus>(json, "status", m_orderStatus);
    m_avgPrice = readStringAsDouble(json, "avgPrice");
    m_origQty = readStringAsDouble(json, "origQty");
    m_executedQty = readStringAsDouble(json, "executedQty");
    m_cumQty = readStringAsDouble(json, "cumQty");
    m_cumQuote = readStringAsDouble(json, "cumQuote");
    readMagicEnum<OrderType>(json, "origType", m_origType);
    readValue<int>(json, "code", m_errCode);
    readValue<std::string>(json, "msg", m_errMsg);
}

nlohmann::json OrdersResponse::toJson() const {
    throw std::runtime_error("Unimplemented: OrdersResponse::toJson()");
}

void OrdersResponse::fromJson(const nlohmann::json &json) {
    m_responses.clear();

    for (const auto &el: json) {
        OrderResponse orderResponse;
        orderResponse.fromJson(el);
        m_responses.push_back(orderResponse);
    }
}

nlohmann::json Candle::toJson() const {
    nlohmann::json json;
    json.push_back(m_openTime);
    json.push_back(std::to_string(m_open));
    json.push_back(std::to_string(m_high));
    json.push_back(std::to_string(m_low));
    json.push_back(std::to_string(m_close));
    json.push_back(std::to_string(m_volume));
    json.push_back(m_closeTime);
    json.push_back(std::to_string(m_quoteVolume));
    json.push_back(m_numberOfTrades);
    json.push_back(std::to_string(m_takerBuyVolume));
    json.push_back(std::to_string(m_takerQuoteVolume));
    json.push_back(m_ignore);
    return json;
}

void Candle::fromJson(const nlohmann::json &json) {
    m_openTime = json[0];
    m_open = stod(json[1].get<std::string>());
    m_high = stod(json[2].get<std::string>());
    m_low = stod(json[3].get<std::string>());
    m_close = stod(json[4].get<std::string>());
    m_volume = stod(json[5].get<std::string>());
    m_closeTime = json[6];
    m_quoteVolume = stod(json[7].get<std::string>());
    m_numberOfTrades = json[8];
    m_takerBuyVolume = stod(json[9].get<std::string>());
    m_takerQuoteVolume = stod(json[10].get<std::string>());
    m_ignore = json[11];
}

nlohmann::json CandlesResponse::toJson() const {
    throw std::runtime_error("Unimplemented: CandlesResponse::toJson()");
}

void CandlesResponse::fromJson(const nlohmann::json &json) {
    m_candles.clear();

    for (const auto &el: json) {
        Candle candle;
        candle.fromJson(el);
        m_candles.push_back(candle);
    }
}

nlohmann::json Position::toJson() const {
    throw std::runtime_error("Unimplemented: Position::toJson()");
}

void Position::fromJson(const nlohmann::json &json) {
    m_entryPrice = readStringAsDouble(json, "entryPrice");
    readValue<std::string>(json, "marginType", m_marginType);

    std::string isAutoAddMarginStr;
    readValue<std::string>(json, "isAutoAddMargin", isAutoAddMarginStr);
    m_isAutoAddMargin = string2bool(isAutoAddMarginStr);

    m_isolatedMargin = readStringAsDouble(json, "isolatedMargin");
    m_leverage = readStringAsDouble(json, "leverage");
    m_liquidationPrice = readStringAsDouble(json, "liquidationPrice");
    m_markPrice = readStringAsDouble(json, "markPrice");
    m_maxNotionalValue = readStringAsDouble(json, "maxNotionalValue");
    m_positionAmt = readStringAsDouble(json, "positionAmt");
    readValue<std::string>(json, "symbol", m_symbol);
    m_unRealizedProfit = readStringAsDouble(json, "unRealizedProfit");
    readMagicEnum<PositionSide>(json, "positionSide", m_positionSide);
    readValue<std::int64_t>(json, "updateTime", m_updateTime);
}

nlohmann::json Filter::toJson() const {
    throw std::runtime_error("Unimplemented: Filter::toJson()");
}

void Filter::fromJson(const nlohmann::json &json) {
    readMagicEnum<SymbolFilter>(json, "filterType", m_filterType);
    m_maxPrice = readStringAsDouble(json, "maxPrice");
    m_minPrice = readStringAsDouble(json, "minPrice");
    m_tickSize = readStringAsDouble(json, "tickSize");
    m_minQty = readStringAsDouble(json, "minQty");
    m_maxQty = readStringAsDouble(json, "maxQty");
    m_stepSize = readStringAsDouble(json, "stepSize");
    readValue<int64_t>(json, "limit", m_limit);
    m_multiplierUp = readStringAsDouble(json, "multiplierUp");
    m_multiplierDown = readStringAsDouble(json, "multiplierDown");
    m_multiplierDecimal = readStringAsDouble(json, "multiplierDecimal");
    m_notional = readStringAsDouble(json, "notional");
}

nlohmann::json Symbol::toJson() const {
    throw std::runtime_error("Unimplemented: Symbol::toJson()");
}

void Symbol::fromJson(const nlohmann::json &json) {
    readValue<std::string>(json, "symbol", m_symbol);
    readValue<std::string>(json, "pair", m_pair);
    readValue<std::string>(json, "contractType", m_contractType);
    readValue<int64_t>(json, "deliveryDate", m_deliveryDate);
    readValue<int64_t>(json, "onboardDate", m_onboardDate);
    readMagicEnum<ContractStatus>(json, "status", m_status);
    m_maintMarginPercent = readStringAsDouble(json, "maintMarginPercent");
    m_requiredMarginPercent = readStringAsDouble(json, "requiredMarginPercent");
    readValue<std::string>(json, "baseAsset", m_baseAsset);
    readValue<std::string>(json, "quoteAsset", m_quoteAsset);
    readValue<std::string>(json, "marginAsset", m_marginAsset);
    readValue<int>(json, "pricePrecision", m_pricePrecision);
    readValue<int>(json, "quantityPrecision", m_quantityPrecision);
    readValue<int>(json, "baseAssetPrecision", m_baseAssetPrecision);
    readValue<int>(json, "quotePrecision", m_quotePrecision);
    readValue<std::string>(json, "underlyingType", m_underlyingType);

    m_underlyingSubType.clear();
    for (const auto &el: json["underlyingSubType"]) {
        m_underlyingSubType.push_back(el);
    }

    m_filters.clear();
    for (const auto &el: json["filters"]) {
        Filter filter;
        filter.fromJson(el);
        m_filters.push_back(filter);
    }

    readValue<int64_t>(json, "settlePlan", m_settlePlan);
    m_triggerProtect = readStringAsDouble(json, "triggerProtect");
}

nlohmann::json RateLimit::toJson() const {
    throw std::runtime_error("Unimplemented: RateLimit::toJson()");
}

void RateLimit::fromJson(const nlohmann::json &json) {
    readMagicEnum<RateLimitInterval>(json, "interval", m_interval);
    readValue<int64_t>(json, "intervalNum", m_intervalNum);
    readValue<int64_t>(json, "limit", m_limit);
    readMagicEnum<RateLimitType>(json, "rateLimitType", m_rateLimitType);
}

nlohmann::json Exchange::toJson() const {
    throw std::runtime_error("Unimplemented: Exchange::toJson()");
}

void Exchange::fromJson(const nlohmann::json &json) {
    m_rateLimits.clear();
    m_assets.clear();
    m_symbols.clear();

    for (const auto &el: json["rateLimits"]) {
        RateLimit rateLimit;
        rateLimit.fromJson(el);
        m_rateLimits.push_back(rateLimit);
    }

    for (const auto &el: json["assets"]) {
        Asset asset;
        asset.fromJson(el);
        m_assets.push_back(asset);
    }

    for (const auto &el: json["symbols"]) {
        Symbol symbol;
        symbol.fromJson(el);
        m_symbols.push_back(symbol);
    }

    readValue<int64_t>(json, "serverTime", m_serverTime);
    readValue<std::string>(json, "timezone", m_timezone);
}

nlohmann::json DownloadId::toJson() const {
    throw std::runtime_error("Unimplemented: DownloadId::toJson()");
}

void DownloadId::fromJson(const nlohmann::json &json) {
    readValue<int64_t>(json, "avgCostTimestampOfLast30d", m_avgCostTimestampOfLast30d);
    readValue<std::string>(json, "downloadId", m_downloadId);
}

nlohmann::json Income::toJson() const {
    throw std::runtime_error("Unimplemented: Income::toJson()");
}

void Income::fromJson(const nlohmann::json &json) {
    readValue<std::string>(json, "symbol", m_symbol);
    readMagicEnum<IncomeType>(json, "incomeType", m_incomeType);
    m_income = readStringAsDouble(json, "income");
    readValue<std::string>(json, "asset", m_asset);
    readValue<std::string>(json, "info", m_info);
    readValue<int64_t>(json, "time", m_time);
    readValue<int64_t>(json, "tranId", m_tranId);
    readValue<std::string>(json, "tradeId", m_tradeId);
}

nlohmann::json Incomes::toJson() const {
    throw std::runtime_error("Unimplemented: Incomes::toJson()");
}

void Incomes::fromJson(const nlohmann::json &json) {
    m_incomes.clear();

    for (const auto &el: json) {
        Income income;
        income.fromJson(el);
        m_incomes.push_back(income);
    }
}

nlohmann::json PositionRisk::toJson() const {
    nlohmann::json json;

    json["symbol"] = m_symbol;
    json["entryPrice"] = m_entryPrice;
    json["marginType"] = magic_enum::enum_name(m_marginType);
    json["isAutoAddMargin"] = m_isAutoAddMargin;
    json["isolatedMargin"] = m_isolatedMargin;
    json["leverage"] = m_leverage;
    json["liquidationPrice"] = m_liquidationPrice;
    json["markPrice"] = m_markPrice;
    json["maxNotionalValue"] = m_maxNotionalValue;
    json["positionAmt"] = m_positionAmt;
    json["notional"] = m_notional;
    json["isolatedWallet"] = m_isolatedWallet;
    json["unRealizedProfit"] = m_unRealizedProfit;
    json["positionSide"] = magic_enum::enum_name(m_positionSide);
    json["updateTime"] = m_updateTime;

    return json;
}

void PositionRisk::fromJson(const nlohmann::json &json) {
    readValue<std::string>(json, "symbol", m_symbol);
    m_entryPrice = readStringAsDouble(json, "entryPrice");
    readMagicEnum<MarginType>(json, "marginType", m_marginType);

    std::string isAutoAddMarginStr;
    readValue<std::string>(json, "isAutoAddMargin", isAutoAddMarginStr);
    m_isAutoAddMargin = string2bool(isAutoAddMarginStr);

    m_isolatedMargin = readStringAsDouble(json, "isolatedMargin");
    m_leverage = readStringAsInt(json, "leverage");
    m_liquidationPrice = readStringAsDouble(json, "liquidationPrice");
    m_markPrice = readStringAsDouble(json, "markPrice");
    m_maxNotionalValue = readStringAsDouble(json, "maxNotionalValue");
    m_positionAmt = readStringAsDouble(json, "positionAmt");
    m_notional = readStringAsDouble(json, "notional");
    m_isolatedWallet = readStringAsDouble(json, "isolatedWallet");
    m_unRealizedProfit = readStringAsDouble(json, "unRealizedProfit");
    readMagicEnum<PositionSide>(json, "positionSide", m_positionSide);
    readValue<std::int64_t>(json, "updateTime", m_updateTime);
}

nlohmann::json OpenInterest::toJson() const {
    throw std::runtime_error("Unimplemented: OpenInterest::toJson()");
}

void OpenInterest::fromJson(const nlohmann::json &json) {
    readValue<std::string>(json, "symbol", m_symbol);
    m_openInterest = readStringAsDouble(json, "openInterest");
    readValue<std::int64_t>(json, "time", m_time);
}

nlohmann::json LongShortRatio::toJson() const {
    throw std::runtime_error("Unimplemented: LongShortRatio::toJson()");
}

void LongShortRatio::fromJson(const nlohmann::json &json) {
    readValue<std::string>(json, "symbol", m_symbol);
    m_longShortRatio = readStringAsDouble(json, "longShortRatio");
    m_longAccount = readStringAsDouble(json, "longAccount");
    m_shortAccount = readStringAsDouble(json, "shortAccount");
    readValue<std::int64_t>(json, "timestamp", m_timestamp);
}

nlohmann::json OpenInterestStatistics::toJson() const {
    throw std::runtime_error("Unimplemented: OpenInterestStatistics::toJson()");
}

void OpenInterestStatistics::fromJson(const nlohmann::json &json) {
    readValue<std::string>(json, "symbol", m_symbol);
    m_sumOpenInterest = readStringAsDouble(json, "sumOpenInterest");
    m_sumOpenInterestValue = readStringAsDouble(json, "sumOpenInterestValue");
    readValue<std::int64_t>(json, "timestamp", m_timestamp);
}

nlohmann::json BuySellVolume::toJson() const {
    throw std::runtime_error("Unimplemented: BuySellVolume::toJson()");
}

void BuySellVolume::fromJson(const nlohmann::json &json) {
    m_buySellRatio = readStringAsDouble(json, "buySellRatio");
    m_buyVol = readStringAsDouble(json, "buyVol");
    m_sellVol = readStringAsDouble(json, "sellVol");
    readValue<std::int64_t>(json, "timestamp", m_timestamp);
}
}
