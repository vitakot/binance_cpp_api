/**
Binance Exchange Connector

Licensed under the MIT License <http://opensource.org/licenses/MIT>.
SPDX-License-Identifier: MIT
Copyright (c) 2022 Vitezslav Kot <vitezslav.kot@gmail.com>.
*/

#include <vk/binance/binance_exchange_connector.h>
#include "vk/binance/binance_futures_rest_client.h"
#include "vk/binance/binance_futures_ws_client.h"
#include "vk/binance/binance_ws_stream_manager.h"

namespace vk {
struct BinanceFuturesExchangeConnector::P {
    std::shared_ptr<binance::futures::RESTClient> restClient{};
    std::unique_ptr<binance::futures::WSStreamManager> streamManager{};
    bool m_demo = true;

    static binance::Side generalOrderSideToBinanceOrderSide(const OrderSide &side) {
        switch (side) {
            case OrderSide::Buy:
                return binance::Side::BUY;
            case OrderSide::Sell:
                return binance::Side::SELL;
            default:
                return binance::Side::BUY;
        }
    }

    static binance::futures::OrderType generalOrderTypeToBinanceOrderType(const OrderType &type) {
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

    static binance::TimeInForce generalTImeInForceToBinanceTimeInForce(const TimeInForce &timeInForce) {
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

    static OrderStatus binanceOrderStatusToGeneralOrderStatus(const binance::futures::OrderStatus &status) {
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
}

BinanceFuturesExchangeConnector::~BinanceFuturesExchangeConnector() {
    m_p->streamManager.reset();
    m_p->restClient.reset();
}

std::string BinanceFuturesExchangeConnector::name() const {
    return std::string(magic_enum::enum_name(ExchangeId::BinanceFutures));
}

std::string BinanceFuturesExchangeConnector::version() const {
    return "1.0.4";
}

void BinanceFuturesExchangeConnector::setLoggerCallback(const onLogMessage &onLogMessageCB) {
    m_p->streamManager->setLoggerCallback(onLogMessageCB);
}

void BinanceFuturesExchangeConnector::login(const std::tuple<std::string, std::string, std::string> &credentials) {
    m_p->streamManager.reset();
    m_p->restClient.reset();
    m_p->restClient = std::make_shared<binance::futures::RESTClient>(std::get<0>(credentials),
                                                                     std::get<1>(credentials));
    m_p->streamManager = std::make_unique<binance::futures::WSStreamManager>(m_p->restClient);
}

void BinanceFuturesExchangeConnector::setDemo(const bool demo) {
    m_p->m_demo = demo;
}

bool BinanceFuturesExchangeConnector::isDemo() const {
    return m_p->m_demo;
}

Trade BinanceFuturesExchangeConnector::placeOrder(const Order &order) {
    Trade retVal;

    if (m_p->m_demo) {
        binance::futures::TickerPrice tickerPrice = m_p->restClient->getTickerPrice(order.symbol);
        retVal.fillTime = std::time(nullptr);
        retVal.orderStatus = OrderStatus::Filled;
        retVal.averagePrice = tickerPrice.m_price;
        retVal.filledQuantity = order.quantity;
    } else {
        binance::futures::Order binanceOrder;
        binanceOrder.m_symbol = order.symbol;
        binanceOrder.m_side = P::generalOrderSideToBinanceOrderSide(order.side);
        binanceOrder.m_type = P::generalOrderTypeToBinanceOrderType(order.type);
        binanceOrder.m_timeInForce = P::generalTImeInForceToBinanceTimeInForce(order.timeInForce);
        binanceOrder.m_quantity = order.quantity;
        binanceOrder.m_newOrderRespType = binance::OrderRespType::RESULT;
        binanceOrder.m_newClientOrderId = order.clientOrderId;

        binance::futures::OrderResponse orderResponse = m_p->restClient->sendOrder(binanceOrder);

        retVal.fillTime = orderResponse.m_timestamp;
        retVal.orderStatus = P::binanceOrderStatusToGeneralOrderStatus(orderResponse.m_orderStatus);
        retVal.averagePrice = orderResponse.m_avgPrice;
        retVal.filledQuantity = orderResponse.m_executedQty;
    }

    return retVal;
}

TickerPrice BinanceFuturesExchangeConnector::getSymbolTickerPrice(const std::string &symbol) const {
    TickerPrice retVal;
    const binance::futures::BookTickerPrice bookTickerPrice = m_p->restClient->getBookTickerPrice("BTCUSDT");
    retVal.askPrice = bookTickerPrice.m_askPrice;
    retVal.bidPrice = bookTickerPrice.m_bidPrice;
    return retVal;
}

Balance BinanceFuturesExchangeConnector::getAccountBalance(const std::string &currency) const {
    Balance retVal;

    for (const auto accountBalances = m_p->restClient->getAccountBalances(); const auto &el: accountBalances) {
        if (el.m_asset == currency) {
            retVal.balance = el.m_balance;
        }
    }
    return retVal;
}

FundingRate BinanceFuturesExchangeConnector::getLastFundingRate(const std::string &symbol) const {
    const auto fr = m_p->restClient->getLastFundingRate(symbol);
    return {fr.m_symbol, fr.m_fundingRate, fr.m_fundingTime};
}

std::vector<FundingRate> BinanceFuturesExchangeConnector::getFundingRates(
    const std::string &symbol, const std::int64_t startTime, const std::int64_t endTime) const {
    std::vector<FundingRate> retVal;

    for (const auto fRates = m_p->restClient->getFundingRates(symbol, startTime, endTime); const auto& fr: fRates) {
        retVal.push_back({fr.m_symbol, fr.m_fundingRate, fr.m_fundingTime, {}});
    }
    return retVal;
}
}
