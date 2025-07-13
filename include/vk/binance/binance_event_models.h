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
    EventType m_e{EventType::UNDEFINED}; /// event type
    std::int64_t m_E{}; /// event time

    ~Event() override = default;

    [[nodiscard]] nlohmann::json toJson() const override;

    void fromJson(const nlohmann::json &json) override;
};

struct EventTickPrice final : Event {
    std::int64_t m_u{0}; /// order book updateId
    std::string m_s; /// symbol
    double m_b{}; /// best bid price
    double m_B{}; /// bid qty
    double m_a{}; /// best ask price
    double m_A{}; /// ask qty
    std::int64_t m_T{}; /// transaction time

    EventTickPrice() {
        m_e = EventType::bookTicker;
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
    std::string m_s{}; /// Symbol
    std::string m_c{}; /// Client Order Id
    Side m_S{Side::BUY}; /// Side
    OrderType m_o{OrderType::LIMIT}; /// Order Type
    TimeInForce m_f{TimeInForce::GTC}; /// Time in Force
    double m_q{}; /// Original Quantity
    double m_p{}; /// Original Price
    double m_ap{}; /// Average Price
    double m_sp{}; /// Stop Price. Please ignore with TRAILING_STOP_MARKET order
    ExecutionType m_x{ExecutionType::NEW}; /// Execution Type
    OrderStatus m_X{OrderStatus::NEW}; /// Order Status
    std::int64_t m_i{}; /// Order Id
    double m_l{}; /// Order Last Filled Quantity
    double m_z{}; /// Order Filled Accumulated Quantity
    double m_L{}; /// Last Filled Price
    std::int64_t m_T{}; /// Order Trade Time
    std::int64_t m_t{}; /// Trade Id
    double m_b{}; /// Bids Notional
    double m_a{}; /// Ask Notional
    bool m_m{false}; /// Is this trade the maker side?
    bool m_R{false}; /// Is this reduce only
    WorkingType m_wt{WorkingType::CONTRACT_PRICE}; /// Stop Price Working Type
    OrderType m_ot{OrderType::LIMIT}; /// Original Order Type
    PositionSide m_ps{PositionSide::LONG}; /// Position Side
    bool m_cp{false}; /// If Close-All, pushed with conditional order
    double m_AP{}; /// Activation Price, only pushed with TRAILING_STOP_MARKET order
    double m_cr{}; /// Callback Rate, only pushed with TRAILING_STOP_MARKET order
    double m_rp{}; /// Realized Profit of the trade

    [[nodiscard]] nlohmann::json toJson() const override;

    void fromJson(const nlohmann::json &json) override;
};

struct EventUserData final : IJson {
    std::variant<EventAccountUpdate, EventBalanceUpdate, EventOrderUpdate, Event> eventData;

    [[nodiscard]] nlohmann::json toJson() const override;

    void fromJson(const nlohmann::json &json) override;
};

struct EventAggregatedTrade final : Event {
    std::string m_s{};
    std::int64_t m_a{};
    double m_p{};
    double m_q{};
    std::int64_t m_f{};
    std::int64_t m_l{};
    std::int64_t m_T{};
    bool m_m{false};

    [[nodiscard]] nlohmann::json toJson() const override;

    void fromJson(const nlohmann::json &json) override;
};

struct EventCandlestick final : Event {
    struct Candlestick final : IJson {
        std::int64_t m_t{}; /// Kline start time
        std::int64_t m_T{}; /// Kline close time
        std::string m_s; /// Symbol
        CandleInterval m_i{CandleInterval::_1m}; /// Interval
        std::int64_t m_f{}; /// First trade ID
        std::int64_t m_L{}; /// Last trade ID
        double m_o{}; /// Open price
        double m_h{}; /// High price
        double m_l{}; /// Low price
        double m_c{}; /// Close price
        double m_v{}; /// Base asset volume
        std::int64_t m_n{}; /// Number of trades
        bool m_x = {false}; /// Is this kline closed?
        double m_q{}; /// Quote asset volume
        double m_V{}; /// Taker buy base asset volume
        double m_Q{}; /// Taker buy quote asset volume

        [[nodiscard]] nlohmann::json toJson() const override;

        void fromJson(const nlohmann::json &json) override;
    };

    std::string m_s{};
    Candlestick m_k{};

    [[nodiscard]] nlohmann::json toJson() const override;

    void fromJson(const nlohmann::json &json) override;
};
}
#endif //INCLUDE_VK_BINANCE_EVENT_MODELS_H
