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
    std::shared_ptr<binance::futures::RESTClient> m_restClient{};
    std::unique_ptr<binance::futures::WSStreamManager> m_streamManager{};

    static binance::Side generalOrderSideToBinanceOrderSide(const OrderSide& side) {
        switch (side) {
        case OrderSide::Buy:
            return binance::Side::BUY;
        case OrderSide::Sell:
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
    m_p->m_restClient = std::make_shared<binance::futures::RESTClient>("","");
    m_p->m_streamManager = std::make_unique<binance::futures::WSStreamManager>(m_p->m_restClient);
    m_p->m_restClient->updateExchangeInfo(true);
}

BinanceFuturesExchangeConnector::~BinanceFuturesExchangeConnector() {
    m_p->m_streamManager.reset();
    m_p->m_restClient.reset();
}

std::string BinanceFuturesExchangeConnector::exchangeId() const {
    return std::string(magic_enum::enum_name(ExchangeId::BinanceFutures));
}

std::string BinanceFuturesExchangeConnector::version() const {
    return "1.0.4";
}

void BinanceFuturesExchangeConnector::setLoggerCallback(const onLogMessage& onLogMessageCB) {
    m_p->m_streamManager->setLoggerCallback(onLogMessageCB);
}

void BinanceFuturesExchangeConnector::login(const std::tuple<std::string, std::string, std::string>& credentials) {
    m_p->m_streamManager.reset();
    m_p->m_restClient.reset();
    m_p->m_restClient = std::make_shared<binance::futures::RESTClient>(std::get<0>(credentials),
                                                                     std::get<1>(credentials));
    m_p->m_streamManager = std::make_unique<binance::futures::WSStreamManager>(m_p->m_restClient);
    m_p->m_restClient->updateExchangeInfo(true);
}

Trade BinanceFuturesExchangeConnector::placeOrder(const Order& order) {
    Trade retVal;

    binance::futures::Order binanceOrder;
    binanceOrder.m_symbol = order.symbol;
    binanceOrder.m_side = P::generalOrderSideToBinanceOrderSide(order.side);
    binanceOrder.m_type = P::generalOrderTypeToBinanceOrderType(order.type);
    binanceOrder.m_timeInForce = P::generalTImeInForceToBinanceTimeInForce(order.timeInForce);
    binanceOrder.m_quantity = order.quantity;
    binanceOrder.m_newOrderRespType = binance::OrderRespType::RESULT;
    binanceOrder.m_newClientOrderId = order.clientOrderId;

    binance::futures::OrderResponse orderResponse = m_p->m_restClient->sendOrder(binanceOrder);

    retVal.fillTime = orderResponse.m_timestamp;
    retVal.orderStatus = P::binanceOrderStatusToGeneralOrderStatus(orderResponse.m_orderStatus);
    retVal.averagePrice = orderResponse.m_avgPrice;
    retVal.filledQuantity = orderResponse.m_executedQty;

    return retVal;
}

TickerPrice BinanceFuturesExchangeConnector::getTickerPrice(const std::string& symbol) const {
    TickerPrice retVal;
    const binance::futures::BookTickerPrice bookTickerPrice = m_p->m_restClient->getBookTickerPrice(symbol);
    retVal.askPrice = bookTickerPrice.m_askPrice;
    retVal.bidPrice = bookTickerPrice.m_bidPrice;
    return retVal;
}

std::vector<Ticker> BinanceFuturesExchangeConnector::getTickerInfo(const std::string& symbol) const {
    std::vector<Ticker> retVal;
    std::vector<binance::futures::Symbol> symbolsToSearch;
    const auto exchangeInfo = m_p->m_restClient->getExchangeInfo();

    constexpr auto symbolContract = binance::futures::ContractType::PERPETUAL;
    const auto symbolType = std::string(magic_enum::enum_name(symbolContract));

    for (const auto& el : exchangeInfo.m_symbols) {
        if (el.m_contractType == symbolType && el.m_quoteAsset == "USDT" && el.m_status ==
            binance::ContractStatus::TRADING) {
            symbolsToSearch.push_back(el);
        }
    }

    for (const auto& el : symbolsToSearch) {
        if ((!symbol.empty() && el.m_symbol == symbol) || symbol.empty()) {
            Ticker ticker;
            ticker.marketCategory = MarketCategory::Futures;
            ticker.symbol = el.m_symbol;
            ticker.baseAsset = el.m_baseAsset;
            ticker.marginAsset = el.m_marginAsset;
            ticker.quoteAsset = el.m_quoteAsset;
            ticker.displayName = el.m_pair;
            retVal.push_back(ticker);
        }
    }

    return retVal;
}

Balance BinanceFuturesExchangeConnector::getAccountBalance(const std::string& currency) const {
    Balance retVal;

    for (const auto accountBalances = m_p->m_restClient->getAccountBalances(); const auto& el : accountBalances) {
        if (el.m_asset == currency) {
            retVal.balance = el.m_balance;
        }
    }
    return retVal;
}

FundingRate BinanceFuturesExchangeConnector::getFundingRate(const std::string& symbol) const {
    const auto fr = m_p->m_restClient->getLastFundingRate(symbol);
    return {fr.m_symbol, fr.m_fundingRate, fr.m_fundingTime};
}

std::vector<FundingRate> BinanceFuturesExchangeConnector::getFundingRates() const {
    std::vector<FundingRate> retVal;

    for (const auto& mp : m_p->m_restClient->getMarkPrices()) {
        FundingRate fr;
        fr.symbol = mp.m_symbol;
        fr.fundingRate = mp.m_lastFundingRate;
        fr.fundingTime = mp.m_nextFundingTime;
        retVal.push_back(fr);
    }

    return retVal;
}

std::int64_t BinanceFuturesExchangeConnector::getServerTime() const {
    return m_p->m_restClient->getServerTime();
}
}
