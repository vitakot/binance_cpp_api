/**
Binance Data Models

Licensed under the MIT License <http://opensource.org/licenses/MIT>.
SPDX-License-Identifier: MIT
Copyright (c) 2022 Vitezslav Kot <vitezslav.kot@gmail.com>.
*/

#include "vk/binance/binance_models.h"
#include "vk/utils/utils.h"
#include "vk/utils/json_utils.h"

namespace vk::binance {
nlohmann::json Candle::toJson() const {
    nlohmann::json json;
    json.push_back(openTime);
    json.push_back(std::to_string(open));
    json.push_back(std::to_string(high));
    json.push_back(std::to_string(low));
    json.push_back(std::to_string(close));
    json.push_back(std::to_string(volume));
    json.push_back(closeTime);
    json.push_back(std::to_string(quoteVolume));
    json.push_back(numberOfTrades);
    json.push_back(std::to_string(takerBuyVolume));
    json.push_back(std::to_string(takerQuoteVolume));
    json.push_back(ignore);
    return json;
}

void Candle::fromJson(const nlohmann::json &json) {
    openTime = json[0];
    open = stod(json[1].get<std::string>());
    high = stod(json[2].get<std::string>());
    low = stod(json[3].get<std::string>());
    close = stod(json[4].get<std::string>());
    volume = stod(json[5].get<std::string>());
    closeTime = json[6];
    quoteVolume = stod(json[7].get<std::string>());
    numberOfTrades = json[8];
    takerBuyVolume = stod(json[9].get<std::string>());
    takerQuoteVolume = stod(json[10].get<std::string>());
    ignore = json[11];
}

nlohmann::json CandlesResponse::toJson() const {
    throw std::runtime_error("Unimplemented: CandlesResponse::toJson()");
}

void CandlesResponse::fromJson(const nlohmann::json &json) {
    candles.clear();

    for (const auto &el: json) {
        Candle candle;
        candle.fromJson(el);
        candles.push_back(candle);
    }
}

nlohmann::json ErrorResponse::toJson() const {
    throw std::runtime_error("Unimplemented: ErrorResponse::toJson()");
}

void ErrorResponse::fromJson(const nlohmann::json &json) {
    readValue<int>(json, "code", code);
    readValue<std::string>(json, "msg", msg);
}

nlohmann::json RateLimit::toJson() const {
    throw std::runtime_error("Unimplemented: RateLimit::toJson()");
}

void RateLimit::fromJson(const nlohmann::json &json) {
    readMagicEnum<RateLimitInterval>(json, "interval", interval);
    readValue<int32_t>(json, "intervalNum", intervalNum);
    readValue<int32_t>(json, "limit", limit);
    readMagicEnum<RateLimitType>(json, "rateLimitType", rateLimitType);
}
}

namespace vk::binance::spot {
nlohmann::json Symbol::toJson() const {
    throw std::runtime_error("Unimplemented: Symbol::toJson()");
}

void Symbol::fromJson(const nlohmann::json &json) {
    readValue<std::string>(json, "symbol", symbol);
    readMagicEnum<ContractStatus>(json, "status", status);
    readValue<std::string>(json, "baseAsset", baseAsset);
    readValue<std::string>(json, "quoteAsset", quoteAsset);
}

nlohmann::json Exchange::toJson() const {
    throw std::runtime_error("Unimplemented: Exchange::toJson()");
}

void Exchange::fromJson(const nlohmann::json &json) {
    rateLimits.clear();
    symbols.clear();

    for (const auto &el: json["rateLimits"]) {
        RateLimit rateLimit;
        rateLimit.fromJson(el);
        rateLimits.push_back(rateLimit);
    }

    for (const auto &el: json["symbols"]) {
        Symbol symbol;
        symbol.fromJson(el);
        symbols.push_back(symbol);
    }
}
}

namespace vk::binance::futures {

nlohmann::json FundingRate::toJson() const {
    throw std::runtime_error("Unimplemented: FundingRate::toJson()");
}

void FundingRate::fromJson(const nlohmann::json &json) {
    readValue<std::string>(json, "symbol", symbol);
    readValue<std::int64_t>(json, "fundingTime", fundingTime);
    fundingRate = readStringAsDouble(json, "fundingRate");
}

nlohmann::json FundingRates::toJson() const {
    throw std::runtime_error("Unimplemented: FundingRates::toJson()");
}

void FundingRates::fromJson(const nlohmann::json &json) {
    fundingRates.clear();

    for (const auto &el: json) {
        FundingRate fundingRate;
        fundingRate.fromJson(el);
        fundingRates.push_back(fundingRate);
    }
}

nlohmann::json TickerPrice::toJson() const {
    throw std::runtime_error("Unimplemented: TickerPrice::toJson()");
}

void TickerPrice::fromJson(const nlohmann::json &json) {
    readValue<std::string>(json, "symbol", symbol);
    price = readStringAsDouble(json, "price");
    readValue<std::int64_t>(json, "time", time);
}

nlohmann::json BookTickerPrice::toJson() const {
    throw std::runtime_error("Unimplemented: BookTickerPrice::toJson()");
}

void BookTickerPrice::fromJson(const nlohmann::json &json) {
    readValue<std::string>(json, "symbol", symbol);
    bidPrice = readStringAsDouble(json, "bidPrice");
    askPrice = readStringAsDouble(json, "askPrice");
    bidQty = readStringAsDouble(json, "bidQty");
    askQty = readStringAsDouble(json, "askQty");
    readValue<std::int64_t>(json, "time", time);
}

nlohmann::json MarkPrice::toJson() const {
    throw std::runtime_error("Unimplemented: MarkPrice::toJson()");
}

void MarkPrice::fromJson(const nlohmann::json &json) {
    readValue<std::string>(json, "symbol", symbol);
    readValue<std::int64_t>(json, "nextFundingTime", nextFundingTime);
    readValue<std::int64_t>(json, "time", time);
    markPrice = readStringAsDouble(json, "markPrice");
    indexPrice = readStringAsDouble(json, "indexPrice");
    estimatedSettlePrice = readStringAsDouble(json, "estimatedSettlePrice");
    lastFundingRate = readStringAsDouble(json, "lastFundingRate");
    interestRate = readStringAsDouble(json, "interestRate");
}

nlohmann::json MarkPrices::toJson() const {
    throw std::runtime_error("Unimplemented: MarkPrices::toJson()");
}

void MarkPrices::fromJson(const nlohmann::json &json) {
    markPrices.clear();

    for (const auto &el: json) {
        MarkPrice markPrice;
        markPrice.fromJson(el);
        markPrices.push_back(markPrice);
    }
}

nlohmann::json Asset::toJson() const {
    throw std::runtime_error("Unimplemented: Asset::toJson()");
}

void Asset::fromJson(const nlohmann::json &json) {
    readValue<std::string>(json, "asset", asset);
    walletBalance = readStringAsDouble(json, "walletBalance");
    unrealizedProfit = readStringAsDouble(json, "unrealizedProfit");
    marginBalance = readStringAsDouble(json, "marginBalance");
    maintMargin = readStringAsDouble(json, "maintMargin");
    initialMargin = readStringAsDouble(json, "initialMargin");
    positionInitialMargin = readStringAsDouble(json, "positionInitialMargin");
    openOrderInitialMargin = readStringAsDouble(json, "openOrderInitialMargin");
    crossWalletBalance = readStringAsDouble(json, "crossWalletBalance");
    crossUnPnl = readStringAsDouble(json, "crossUnPnl");
    availableBalance = readStringAsDouble(json, "availableBalance");
    maxWithdrawAmount = readStringAsDouble(json, "maxWithdrawAmount");
    readValue<bool>(json, "marginAvailable", marginAvailable);
    readValue<int64_t>(json, "updateTime", updateTime);
    autoAssetExchange = readStringAsDouble(json, "autoAssetExchange");
}

nlohmann::json Account::toJson() const {
    throw std::runtime_error("Unimplemented: Account::toJson()");
}

void Account::fromJson(const nlohmann::json &json) {
    readValue<int>(json, "feeTier", feeTier);
    readValue<bool>(json, "canTrade", canTrade);
    readValue<bool>(json, "canDeposit", canDeposit);
    readValue<bool>(json, "canWithdraw", canWithdraw);
    readValue<int64_t>(json, "updateTime", updateTime);
    totalInitialMargin = readStringAsDouble(json, "totalInitialMargin");
    totalMaintMargin = readStringAsDouble(json, "totalMaintMargin");
    totalWalletBalance = readStringAsDouble(json, "totalWalletBalance");
    totalUnrealizedProfit = readStringAsDouble(json, "totalUnrealizedProfit");
    totalMarginBalance = readStringAsDouble(json, "totalMarginBalance");
    totalPositionInitialMargin = readStringAsDouble(json, "totalPositionInitialMargin");
    totalOpenOrderInitialMargin = readStringAsDouble(json, "totalOpenOrderInitialMargin");
    totalCrossWalletBalance = readStringAsDouble(json, "totalCrossWalletBalance");
    totalCrossUnPnl = readStringAsDouble(json, "totalCrossUnPnl");
    availableBalance = readStringAsDouble(json, "availableBalance");
    maxWithdrawAmount = readStringAsDouble(json, "maxWithdrawAmount");
    readValue<int>(json, "tradeGroupId", tradeGroupId);

    assets.clear();

    for (const auto &el: json["assets"]) {
        Asset asset;
        asset.fromJson(el);
        assets.push_back(asset);
    }
}

nlohmann::json AccountBalance::toJson() const {
    throw std::runtime_error("Unimplemented: AccountBalance::toJson()");
}

void AccountBalance::fromJson(const nlohmann::json &json) {
    readValue<std::string>(json, "accountAlias", accountAlias);
    readValue<std::string>(json, "asset", asset);
    balance = readStringAsDouble(json, "balance");
    crossWalletBalance = readStringAsDouble(json, "crossWalletBalance");
    crossUnPnl = readStringAsDouble(json, "crossUnPnl");
    availableBalance = readStringAsDouble(json, "availableBalance");
    maxWithdrawAmount = readStringAsDouble(json, "maxWithdrawAmount");
    readValue<bool>(json, "marginAvailable", marginAvailable);
    readValue<std::int64_t>(json, "updateTime", updateTime);
}

nlohmann::json Order::toJson() const {
    nlohmann::json json;
    json["symbol"] = symbol;
    json["side"] = magic_enum::enum_name(side);
    json["positionSide"] = magic_enum::enum_name(positionSide);
    json["type"] = magic_enum::enum_name(type);
    json["orderId"] = std::to_string(orderId);

    if (!newClientOrderId.empty()) {
        json["newClientOrderId"] = newClientOrderId;
    }

    json["newOrderRespType"] = magic_enum::enum_name(newOrderRespType);

    if (!closePosition) {
        json["reduceOnly"] = fmt::format("{}", reduceOnly);
    }

    if (type == OrderType::LIMIT) {
        json["timeInForce"] = magic_enum::enum_name(timeInForce);
        json["quantity"] = formatDouble(quantityPrecision, quantity);
        json["price"] = formatDouble(pricePrecision, price);
    } else if (type == OrderType::MARKET) {
        json["quantity"] = formatDouble(quantityPrecision, quantity);
    } else if (type == OrderType::STOP ||
               type == OrderType::TAKE_PROFIT) {
        json["quantity"] = formatDouble(quantityPrecision, quantity);
        json["price"] = formatDouble(pricePrecision, price);
        json["stopPrice"] = formatDouble(pricePrecision, stopPrice);
    } else if (type == OrderType::STOP_MARKET ||
               type == OrderType::TAKE_PROFIT_MARKET) {
        json["quantity"] = formatDouble(quantityPrecision, quantity);
        json["stopPrice"] = formatDouble(pricePrecision, stopPrice);
        json["priceProtect"] = fmt::format("{}", priceProtect);
        json["closePosition"] = fmt::format("{}", closePosition);
    } else if (type == OrderType::TRAILING_STOP_MARKET) {
        json["quantity"] = formatDouble(quantityPrecision, quantity);
        json["callbackRate"] = std::to_string(callbackRate);
        json["activationPrice"] = formatDouble(pricePrecision, activationPrice);
    }

    json["selfTradePreventionMode"] = magic_enum::enum_name(selfTradePreventionMode);

    return json;
}

void Order::fromJson(const nlohmann::json &json) {
    readValue<std::string>(json, "symbol", symbol);
    readMagicEnum<Side>(json, "side", side);
    price = readStringAsDouble(json, "price");
    readMagicEnum<PositionSide>(json, "positionSide", positionSide);
    readMagicEnum<OrderType>(json, "type", type);
    readMagicEnum<TimeInForce>(json, "timeInForce", timeInForce);
    quantity = readStringAsDouble(json, "quantity");
    readValue<std::string>(json, "newClientOrderId", newClientOrderId);
    stopPrice = readStringAsDouble(json, "stopPrice");
    readValue<int64_t>(json, "timestamp", timestamp);
    readValue<int64_t>(json, "orderId", orderId);
    readValue<bool>(json, "reduceOnly", reduceOnly);
    readValue<bool>(json, "closePosition", closePosition);
    activationPrice = readStringAsDouble(json, "activationPrice");
    callbackRate = readStringAsDouble(json, "callbackRate");
    readMagicEnum<WorkingType>(json, "workingType", workingType);
    readValue<bool>(json, "priceProtect", priceProtect);
    readMagicEnum<OrderRespType>(json, "newOrderRespType", newOrderRespType);
    readMagicEnum<SelfTradePreventionMode>(json, "selfTradePreventionMode", selfTradePreventionMode);
}

nlohmann::json OrderResponse::toJson() const {
    nlohmann::json json = Order::toJson();

    json["orderId"] = orderId;
    json["clientOrderId"] = clientOrderId;
    json["status"] = magic_enum::enum_name(orderStatus);
    json["avgPrice"] = std::to_string(avgPrice);
    json["origQty"] = std::to_string(origQty);
    json["executedQty"] = std::to_string(executedQty);
    json["cumQty"] = std::to_string(cumQty);
    json["cumQuote"] = std::to_string(cumQuote);
    json["origType"] = magic_enum::enum_name(origType);
    return json;
}

void OrderResponse::fromJson(const nlohmann::json &json) {
    Order::fromJson(json);

    readValue<int64_t>(json, "orderId", orderId);
    readValue<std::string>(json, "clientOrderId", clientOrderId);
    readMagicEnum<OrderStatus>(json, "status", orderStatus);
    avgPrice = readStringAsDouble(json, "avgPrice");
    origQty = readStringAsDouble(json, "origQty");
    executedQty = readStringAsDouble(json, "executedQty");
    cumQty = readStringAsDouble(json, "cumQty");
    cumQuote = readStringAsDouble(json, "cumQuote");
    readMagicEnum<OrderType>(json, "origType", origType);
    readValue<int>(json, "code", errCode);
    readValue<std::string>(json, "msg", errMsg);
}

nlohmann::json OrdersResponse::toJson() const {
    throw std::runtime_error("Unimplemented: OrdersResponse::toJson()");
}

void OrdersResponse::fromJson(const nlohmann::json &json) {
    responses.clear();

    for (const auto &el: json) {
        OrderResponse orderResponse;
        orderResponse.fromJson(el);
        responses.push_back(orderResponse);
    }
}

nlohmann::json Position::toJson() const {
    throw std::runtime_error("Unimplemented: Position::toJson()");
}

void Position::fromJson(const nlohmann::json &json) {
    entryPrice = readStringAsDouble(json, "entryPrice");
    readValue<std::string>(json, "marginType", marginType);

    std::string isAutoAddMarginStr;
    readValue<std::string>(json, "isAutoAddMargin", isAutoAddMarginStr);
    isAutoAddMargin = string2bool(isAutoAddMarginStr);

    isolatedMargin = readStringAsDouble(json, "isolatedMargin");
    leverage = readStringAsDouble(json, "leverage");
    liquidationPrice = readStringAsDouble(json, "liquidationPrice");
    markPrice = readStringAsDouble(json, "markPrice");
    maxNotionalValue = readStringAsDouble(json, "maxNotionalValue");
    positionAmt = readStringAsDouble(json, "positionAmt");
    readValue<std::string>(json, "symbol", symbol);
    unRealizedProfit = readStringAsDouble(json, "unRealizedProfit");
    readMagicEnum<PositionSide>(json, "positionSide", positionSide);
    readValue<std::int64_t>(json, "updateTime", updateTime);
}

nlohmann::json Filter::toJson() const {
    throw std::runtime_error("Unimplemented: Filter::toJson()");
}

void Filter::fromJson(const nlohmann::json &json) {
    readMagicEnum<SymbolFilter>(json, "filterType", filterType);
    maxPrice = readStringAsDouble(json, "maxPrice");
    minPrice = readStringAsDouble(json, "minPrice");
    tickSize = readStringAsDouble(json, "tickSize");
    minQty = readStringAsDouble(json, "minQty");
    maxQty = readStringAsDouble(json, "maxQty");
    stepSize = readStringAsDouble(json, "stepSize");
    readValue<int64_t>(json, "limit", limit);
    multiplierUp = readStringAsDouble(json, "multiplierUp");
    multiplierDown = readStringAsDouble(json, "multiplierDown");
    multiplierDecimal = readStringAsDouble(json, "multiplierDecimal");
    notional = readStringAsDouble(json, "notional");
}

nlohmann::json Symbol::toJson() const {
    throw std::runtime_error("Unimplemented: Symbol::toJson()");
}

void Symbol::fromJson(const nlohmann::json &json) {
    readValue<std::string>(json, "symbol", symbol);
    readValue<std::string>(json, "pair", pair);
    readValue<std::string>(json, "contractType", contractType);
    readValue<int64_t>(json, "deliveryDate", deliveryDate);
    readValue<int64_t>(json, "onboardDate", onboardDate);
    readMagicEnum<ContractStatus>(json, "status", status);
    maintMarginPercent = readStringAsDouble(json, "maintMarginPercent");
    requiredMarginPercent = readStringAsDouble(json, "requiredMarginPercent");
    readValue<std::string>(json, "baseAsset", baseAsset);
    readValue<std::string>(json, "quoteAsset", quoteAsset);
    readValue<std::string>(json, "marginAsset", marginAsset);
    readValue<int>(json, "pricePrecision", pricePrecision);
    readValue<int>(json, "quantityPrecision", quantityPrecision);
    readValue<int>(json, "baseAssetPrecision", baseAssetPrecision);
    readValue<int>(json, "quotePrecision", quotePrecision);
    readValue<std::string>(json, "underlyingType", underlyingType);

    underlyingSubType.clear();
    for (const auto &el: json["underlyingSubType"]) {
        underlyingSubType.push_back(el);
    }

    filters.clear();
    for (const auto &el: json["filters"]) {
        Filter filter;
        filter.fromJson(el);
        filters.push_back(filter);
    }

    readValue<int64_t>(json, "settlePlan", settlePlan);
    triggerProtect = readStringAsDouble(json, "triggerProtect");
}

nlohmann::json Exchange::toJson() const {
    throw std::runtime_error("Unimplemented: Exchange::toJson()");
}

void Exchange::fromJson(const nlohmann::json &json) {
    rateLimits.clear();
    assets.clear();
    symbols.clear();

    for (const auto &el: json["rateLimits"]) {
        RateLimit rateLimit;
        rateLimit.fromJson(el);
        rateLimits.push_back(rateLimit);
    }

    for (const auto &el: json["assets"]) {
        Asset asset;
        asset.fromJson(el);
        assets.push_back(asset);
    }

    for (const auto &el: json["symbols"]) {
        Symbol symbol;
        symbol.fromJson(el);
        symbols.push_back(symbol);
    }

    readValue<int64_t>(json, "serverTime", serverTime);
    readValue<std::string>(json, "timezone", timezone);
}

nlohmann::json DownloadId::toJson() const {
    throw std::runtime_error("Unimplemented: DownloadId::toJson()");
}

void DownloadId::fromJson(const nlohmann::json &json) {
    readValue<int64_t>(json, "avgCostTimestampOfLast30d", avgCostTimestampOfLast30d);
    readValue<std::string>(json, "downloadId", downloadId);
}

nlohmann::json Income::toJson() const {
    throw std::runtime_error("Unimplemented: Income::toJson()");
}

void Income::fromJson(const nlohmann::json &json) {
    readValue<std::string>(json, "symbol", symbol);
    readMagicEnum<IncomeType>(json, "incomeType", incomeType);
    income = readStringAsDouble(json, "income");
    readValue<std::string>(json, "asset", asset);
    readValue<std::string>(json, "info", info);
    readValue<int64_t>(json, "time", time);
    readValue<int64_t>(json, "tranId", tranId);
    readValue<std::string>(json, "tradeId", tradeId);
}

nlohmann::json Incomes::toJson() const {
    throw std::runtime_error("Unimplemented: Incomes::toJson()");
}

void Incomes::fromJson(const nlohmann::json &json) {
    incomes.clear();

    for (const auto &el: json) {
        Income income;
        income.fromJson(el);
        incomes.push_back(income);
    }
}

nlohmann::json PositionRisk::toJson() const {
    nlohmann::json json;

    json["symbol"] = symbol;
    json["entryPrice"] = entryPrice;
    json["marginType"] = magic_enum::enum_name(marginType);
    json["isAutoAddMargin"] = isAutoAddMargin;
    json["isolatedMargin"] = isolatedMargin;
    json["leverage"] = leverage;
    json["liquidationPrice"] = liquidationPrice;
    json["markPrice"] = markPrice;
    json["maxNotionalValue"] = maxNotionalValue;
    json["positionAmt"] = positionAmt;
    json["notional"] = notional;
    json["isolatedWallet"] = isolatedWallet;
    json["unRealizedProfit"] = unRealizedProfit;
    json["positionSide"] = magic_enum::enum_name(positionSide);
    json["updateTime"] = updateTime;

    return json;
}

void PositionRisk::fromJson(const nlohmann::json &json) {
    readValue<std::string>(json, "symbol", symbol);
    entryPrice = readStringAsDouble(json, "entryPrice");
    readMagicEnum<MarginType>(json, "marginType", marginType);

    std::string isAutoAddMarginStr;
    readValue<std::string>(json, "isAutoAddMargin", isAutoAddMarginStr);
    isAutoAddMargin = string2bool(isAutoAddMarginStr);

    isolatedMargin = readStringAsDouble(json, "isolatedMargin");
    leverage = readStringAsInt(json, "leverage");
    liquidationPrice = readStringAsDouble(json, "liquidationPrice");
    markPrice = readStringAsDouble(json, "markPrice");
    maxNotionalValue = readStringAsDouble(json, "maxNotionalValue");
    positionAmt = readStringAsDouble(json, "positionAmt");
    notional = readStringAsDouble(json, "notional");
    isolatedWallet = readStringAsDouble(json, "isolatedWallet");
    unRealizedProfit = readStringAsDouble(json, "unRealizedProfit");
    readMagicEnum<PositionSide>(json, "positionSide", positionSide);
    readValue<std::int64_t>(json, "updateTime", updateTime);
}

nlohmann::json OpenInterest::toJson() const {
    throw std::runtime_error("Unimplemented: OpenInterest::toJson()");
}

void OpenInterest::fromJson(const nlohmann::json &json) {
    readValue<std::string>(json, "symbol", symbol);
    openInterest = readStringAsDouble(json, "openInterest");
    readValue<std::int64_t>(json, "time", time);
}

nlohmann::json LongShortRatio::toJson() const {
    throw std::runtime_error("Unimplemented: LongShortRatio::toJson()");
}

void LongShortRatio::fromJson(const nlohmann::json &json) {
    readValue<std::string>(json, "symbol", symbol);
    longShortRatio = readStringAsDouble(json, "longShortRatio");
    longAccount = readStringAsDouble(json, "longAccount");
    shortAccount = readStringAsDouble(json, "shortAccount");
    readValue<std::int64_t>(json, "timestamp", timestamp);
}

nlohmann::json OpenInterestStatistics::toJson() const {
    throw std::runtime_error("Unimplemented: OpenInterestStatistics::toJson()");
}

void OpenInterestStatistics::fromJson(const nlohmann::json &json) {
    readValue<std::string>(json, "symbol", symbol);
    sumOpenInterest = readStringAsDouble(json, "sumOpenInterest");
    sumOpenInterestValue = readStringAsDouble(json, "sumOpenInterestValue");
    readValue<std::int64_t>(json, "timestamp", timestamp);
}

nlohmann::json BuySellVolume::toJson() const {
    throw std::runtime_error("Unimplemented: BuySellVolume::toJson()");
}

void BuySellVolume::fromJson(const nlohmann::json &json) {
    buySellRatio = readStringAsDouble(json, "buySellRatio");
    buyVol = readStringAsDouble(json, "buyVol");
    sellVol = readStringAsDouble(json, "sellVol");
    readValue<std::int64_t>(json, "timestamp", timestamp);
}
}
