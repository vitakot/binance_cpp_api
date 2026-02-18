/**
Binance Exchange Connector

Licensed under the MIT License <http://opensource.org/licenses/MIT>.
SPDX-License-Identifier: MIT
Copyright (c) 2022 Vitezslav Kot <vitezslav.kot@gmail.com>.
*/

#include <vk/binance/binance_futures_exchange_connector.h>
#include "vk/binance/binance_futures_rest_client.h"
#include "vk/binance/binance_futures_ws_client.h"
#include "vk/binance/binance_ws_stream_manager.h"

namespace vk {
struct BinanceFuturesExchangeConnector::P {
    std::shared_ptr<binance::futures::RESTClient> restClient{};
    std::unique_ptr<binance::futures::WSStreamManager> streamManager{};

    static binance::Side generalOrderSideToBinanceOrderSide(const Side& side) {
        switch (side) {
        case Side::Buy:
            return binance::Side::BUY;
        case Side::Sell:
            return binance::Side::SELL;
        default:
            return binance::Side::BUY;
        }
    }

    static binance::futures::OrderType generalOrderTypeToBinanceOrderType(const OrderType& type) {
        switch (type) {
        case OrderType::Market:
            return binance::futures::OrderType::MARKET;
        case OrderType::Limit:
            return binance::futures::OrderType::LIMIT;
        case OrderType::Stop:
            return binance::futures::OrderType::STOP_MARKET;
        case OrderType::StopLimit:
            return binance::futures::OrderType::STOP;
        default:
            return binance::futures::OrderType::MARKET;
        }
    }

    static binance::TimeInForce generalTImeInForceToBinanceTimeInForce(const TimeInForce& timeInForce) {
        switch (timeInForce) {
        case TimeInForce::GTC:
            return binance::TimeInForce::GTC;
        case TimeInForce::IOC:
            return binance::TimeInForce::IOC;
        case TimeInForce::FOK:
            return binance::TimeInForce::FOK;
        default:
            return binance::TimeInForce::GTC;
        }
    }

    static OrderStatus binanceOrderStatusToGeneralOrderStatus(const binance::futures::OrderStatus& status) {
        switch (status) {
        case binance::futures::OrderStatus::NEW:
            return OrderStatus::New;
        case binance::futures::OrderStatus::PARTIALLY_FILLED:
        case binance::futures::OrderStatus::FILLED:
            return OrderStatus::Filled;
        case binance::futures::OrderStatus::CANCELED:
        case binance::futures::OrderStatus::PENDING_CANCEL:
            return OrderStatus::Cancelled;
        case binance::futures::OrderStatus::REJECTED:
            return OrderStatus::Rejected;
        case binance::futures::OrderStatus::EXPIRED:
            return OrderStatus::Expired;
        default:
            return OrderStatus::Filled;
        }
    }
};

BinanceFuturesExchangeConnector::BinanceFuturesExchangeConnector() : m_p(std::make_unique<P>()) {
    m_p->restClient = std::make_shared<binance::futures::RESTClient>("","");
    m_p->streamManager = std::make_unique<binance::futures::WSStreamManager>(m_p->restClient);
    m_p->restClient->updateExchangeInfo(true);
}

BinanceFuturesExchangeConnector::~BinanceFuturesExchangeConnector() {
    m_p->streamManager.reset();
    m_p->restClient.reset();
}

std::string BinanceFuturesExchangeConnector::exchangeId() const {
    return std::string(magic_enum::enum_name(ExchangeId::BinanceFutures));
}

std::string BinanceFuturesExchangeConnector::version() const {
    return "1.0.4";
}

void BinanceFuturesExchangeConnector::setLoggerCallback(const onLogMessage& onLogMessageCB) {
    m_p->streamManager->setLoggerCallback(onLogMessageCB);
}

void BinanceFuturesExchangeConnector::login(const std::tuple<std::string, std::string, std::string>& credentials) {
    m_p->streamManager.reset();
    m_p->restClient.reset();
    m_p->restClient = std::make_shared<binance::futures::RESTClient>(std::get<0>(credentials),
                                                                     std::get<1>(credentials));
    m_p->streamManager = std::make_unique<binance::futures::WSStreamManager>(m_p->restClient);
    m_p->restClient->updateExchangeInfo(true);
}

Trade BinanceFuturesExchangeConnector::placeOrder(const Order& order) {
    Trade retVal;

    binance::futures::Order binanceOrder;
    binanceOrder.symbol = order.symbol;
    binanceOrder.side = P::generalOrderSideToBinanceOrderSide(order.side);
    binanceOrder.type = P::generalOrderTypeToBinanceOrderType(order.type);
    binanceOrder.timeInForce = P::generalTImeInForceToBinanceTimeInForce(order.timeInForce);
    binanceOrder.quantity = order.quantity;
    binanceOrder.newOrderRespType = binance::OrderRespType::RESULT;
    binanceOrder.newClientOrderId = order.clientOrderId;

    binance::futures::OrderResponse orderResponse = m_p->restClient->sendOrder(binanceOrder);

    retVal.fillTime = orderResponse.timestamp;
    retVal.orderStatus = P::binanceOrderStatusToGeneralOrderStatus(orderResponse.orderStatus);
    retVal.averagePrice = orderResponse.avgPrice;
    retVal.filledQuantity = orderResponse.executedQty;

    return retVal;
}

TickerPrice BinanceFuturesExchangeConnector::getTickerPrice(const std::string& symbol) const {
    TickerPrice retVal;
    const binance::futures::BookTickerPrice bookTickerPrice = m_p->restClient->getBookTickerPrice(symbol);
    retVal.askPrice = bookTickerPrice.askPrice;
    retVal.bidPrice = bookTickerPrice.bidPrice;
    return retVal;
}

std::vector<Symbol> BinanceFuturesExchangeConnector::getSymbolInfo(const std::string& symbol) const {
    std::vector<Symbol> retVal;
    std::vector<binance::futures::Symbol> symbolsToSearch;
    const auto exchangeInfo = m_p->restClient->getExchangeInfo();

    constexpr auto symbolContract = binance::futures::ContractType::PERPETUAL;
    const auto symbolType = std::string(magic_enum::enum_name(symbolContract));

    for (const auto& el : exchangeInfo.symbols) {
        if (el.contractType == symbolType && el.quoteAsset == "USDT" && el.status ==
            binance::ContractStatus::TRADING) {
            symbolsToSearch.push_back(el);
        }
    }

    for (const auto& el : symbolsToSearch) {
        if ((!symbol.empty() && el.symbol == symbol) || symbol.empty()) {
            Symbol ticker;
            ticker.marketCategory = MarketCategory::Futures;
            ticker.symbol = el.symbol;
            ticker.baseAsset = el.baseAsset;
            ticker.marginAsset = el.marginAsset;
            ticker.quoteAsset = el.quoteAsset;
            ticker.displayName = el.pair;
            retVal.push_back(ticker);
        }
    }

    return retVal;
}

Balance BinanceFuturesExchangeConnector::getAccountBalance(const std::string& currency) const {
    Balance retVal;

    for (const auto accountBalances = m_p->restClient->getAccountBalances(); const auto& el : accountBalances) {
        if (el.asset == currency) {
            retVal.balance = el.balance;
        }
    }
    return retVal;
}

FundingRate BinanceFuturesExchangeConnector::getFundingRate(const std::string& symbol) const {
    const auto fr = m_p->restClient->getLastFundingRate(symbol);
    return {fr.symbol, fr.fundingRate, fr.fundingTime};
}

std::vector<FundingRate> BinanceFuturesExchangeConnector::getFundingRates() const {
    std::vector<FundingRate> retVal;

    for (const auto& mp : m_p->restClient->getMarkPrices()) {
        FundingRate fr;
        fr.symbol = mp.symbol;
        fr.fundingRate = mp.lastFundingRate;
        fr.fundingTime = mp.nextFundingTime;
        retVal.push_back(fr);
    }

    return retVal;
}

std::int64_t BinanceFuturesExchangeConnector::getServerTime() const {
    return m_p->restClient->getServerTime();
}

std::vector<Position> BinanceFuturesExchangeConnector::getPositionInfo(const std::string& symbol) const {
    throw std::runtime_error("Unimplemented: BinanceFuturesExchangeConnector::getPositionInfo");
}

std::vector<FundingRate> BinanceFuturesExchangeConnector::getHistoricalFundingRates(const std::string& symbol, std::int64_t startTime, std::int64_t endTime) const {
    throw std::runtime_error("Unimplemented: BinanceFuturesExchangeConnector::getHistoricalFundingRates");
}

std::vector<Candle> BinanceFuturesExchangeConnector::getHistoricalCandles(const std::string& symbol, CandleInterval interval, std::int64_t startTime, std::int64_t endTime) const {
    throw std::runtime_error("Unimplemented: BinanceFuturesExchangeConnector::getHistoricalCandles");
}
} // namespace vk
