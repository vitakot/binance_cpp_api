/**
Binance Data Models

Licensed under the MIT License <http://opensource.org/licenses/MIT>.
SPDX-License-Identifier: MIT
Copyright (c) 2022 Vitezslav Kot <vitezslav.kot@gmail.com>.
*/

#ifndef INCLUDE_VK_BINANCE_EVENT_MODELS_H
#define INCLUDE_VK_BINANCE_EVENT_MODELS_H

#include "vk/interface/i_json.h"
#include "binance_models.h"
#include <nlohmann/json.hpp>
#include <variant>

namespace vk::binance::futures {
/**
 * WebSocket Events. Some are Upper Case, some not - Binance mess
 */
enum class EventType : std::int32_t {
    UNDEFINED,
    MARGIN_CALL,
    ACCOUNT_UPDATE,
    ORDER_TRADE_UPDATE,
    ACCOUNT_CONFIG_UPDATE,
    listenKeyExpired,
    bookTicker,
    aggTrade,
    kline
};

struct Event : IJson {
    EventType e{EventType::UNDEFINED}; /// event type
    std::int64_t E{}; /// event time

    ~Event() override = default;

    [[nodiscard]] nlohmann::json toJson() const override;

    void fromJson(const nlohmann::json &json) override;
};

struct EventTickPrice final : Event {
    std::int64_t u{0}; /// order book updateId
    std::string s; /// symbol
    double b{}; /// best bid price
    double B{}; /// bid qty
    double a{}; /// best ask price
    double A{}; /// ask qty
    std::int64_t T{}; /// transaction time

    EventTickPrice() {
        e = EventType::bookTicker;
    }

    [[nodiscard]] nlohmann::json toJson() const override;

    void fromJson(const nlohmann::json &json) override;
};

struct EventAccountUpdate final : Event {
    [[nodiscard]] nlohmann::json toJson() const override;

    void fromJson(const nlohmann::json &json) override;
};

struct EventBalanceUpdate final : Event {
    [[nodiscard]] nlohmann::json toJson() const override;

    void fromJson(const nlohmann::json &json) override;
};

struct EventOrderUpdate final : Event {
    std::string s{}; /// Symbol
    std::string c{}; /// Client Order Id
    Side S{Side::BUY}; /// Side
    OrderType o{OrderType::LIMIT}; /// Order Type
    TimeInForce f{TimeInForce::GTC}; /// Time in Force
    double q{}; /// Original Quantity
    double p{}; /// Original Price
    double ap{}; /// Average Price
    double sp{}; /// Stop Price. Please ignore with TRAILING_STOP_MARKET order
    ExecutionType x{ExecutionType::NEW}; /// Execution Type
    OrderStatus X{OrderStatus::NEW}; /// Order Status
    std::int64_t i{}; /// Order Id
    double l{}; /// Order Last Filled Quantity
    double z{}; /// Order Filled Accumulated Quantity
    double L{}; /// Last Filled Price
    std::int64_t T{}; /// Order Trade Time
    std::int64_t t{}; /// Trade Id
    double b{}; /// Bids Notional
    double a{}; /// Ask Notional
    bool m{false}; /// Is this trade the maker side?
    bool R{false}; /// Is this reduce only
    WorkingType wt{WorkingType::CONTRACT_PRICE}; /// Stop Price Working Type
    OrderType ot{OrderType::LIMIT}; /// Original Order Type
    PositionSide ps{PositionSide::LONG}; /// Position Side
    bool cp{false}; /// If Close-All, pushed with conditional order
    double AP{}; /// Activation Price, only pushed with TRAILING_STOP_MARKET order
    double cr{}; /// Callback Rate, only pushed with TRAILING_STOP_MARKET order
    double rp{}; /// Realized Profit of the trade

    [[nodiscard]] nlohmann::json toJson() const override;

    void fromJson(const nlohmann::json &json) override;
};

struct EventUserData final : IJson {
    std::variant<EventAccountUpdate, EventBalanceUpdate, EventOrderUpdate, Event> eventData;

    [[nodiscard]] nlohmann::json toJson() const override;

    void fromJson(const nlohmann::json &json) override;
};

struct EventAggregatedTrade final : Event {
    std::string s{};
    std::int64_t a{};
    double p{};
    double q{};
    std::int64_t f{};
    std::int64_t l{};
    std::int64_t T{};
    bool m{false};

    [[nodiscard]] nlohmann::json toJson() const override;

    void fromJson(const nlohmann::json &json) override;
};

struct EventCandlestick final : Event {
    struct Candlestick final : IJson {
        std::int64_t t{}; /// Kline start time
        std::int64_t T{}; /// Kline close time
        std::string s; /// Symbol
        CandleInterval i{CandleInterval::_1m}; /// Interval
        std::int64_t f{}; /// First trade ID
        std::int64_t L{}; /// Last trade ID
        double o{}; /// Open price
        double h{}; /// High price
        double l{}; /// Low price
        double c{}; /// Close price
        double v{}; /// Base asset volume
        std::int64_t n{}; /// Number of trades
        bool x = {false}; /// Is this kline closed?
        double q{}; /// Quote asset volume
        double V{}; /// Taker buy base asset volume
        double Q{}; /// Taker buy quote asset volume

        [[nodiscard]] nlohmann::json toJson() const override;

        void fromJson(const nlohmann::json &json) override;
    };

    std::string s{};
    Candlestick k{};

    [[nodiscard]] nlohmann::json toJson() const override;

    void fromJson(const nlohmann::json &json) override;
};
}
#endif //INCLUDE_VK_BINANCE_EVENT_MODELS_H
