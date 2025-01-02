/**
Binance Data Models

Licensed under the MIT License <http://opensource.org/licenses/MIT>.
SPDX-License-Identifier: MIT
Copyright (c) 2022 Vitezslav Kot <vitezslav.kot@gmail.com>.
*/

#include "vk/binance/binance_event_models.h"
#include "vk/tools/utils.h"
#include "vk/tools//json_utils.h"

namespace vk::binance::futures {
nlohmann::json Event::toJson() const {
    nlohmann::json json;
    json["e"] = m_e;
    json["E"] = m_E;
    return json;
}

void Event::fromJson(const nlohmann::json &json) {
    readMagicEnum<EventType>(json, "e", m_e);
    readValue<std::int64_t>(json, "E", m_E);
}

nlohmann::json EventTickPrice::toJson() const {
    nlohmann::json json = Event::toJson();
    json["u"] = m_u;
    json["s"] = m_s;
    json["b"] = std::to_string(m_b);
    json["B"] = std::to_string(m_B);
    json["a"] = std::to_string(m_a);
    json["A"] = std::to_string(m_A);
    return json;
}

void EventTickPrice::fromJson(const nlohmann::json &json) {
    Event::fromJson(json);

    readValue<std::int64_t>(json, "u", m_u);
    readValue<std::int64_t>(json, "T", m_T);
    readValue<std::string>(json, "s", m_s);

    m_b = readStringAsDouble(json, "b");
    m_B = readStringAsDouble(json, "B");
    m_a = readStringAsDouble(json, "a");
    m_A = readStringAsDouble(json, "A");
}

nlohmann::json EventAccountUpdate::toJson() const {
    throw std::runtime_error("Unimplemented: EventAccountUpdate::toJson()");
}

void EventAccountUpdate::fromJson(const nlohmann::json &json) {
    Event::fromJson(json);
}

nlohmann::json EventBalanceUpdate::toJson() const {
    throw std::runtime_error("Unimplemented: EventBalanceUpdate::toJson()");
}

void EventBalanceUpdate::fromJson(const nlohmann::json &json) {
    Event::fromJson(json);
}

nlohmann::json EventOrderUpdate::toJson() const {
    nlohmann::json json = Event::toJson();
    json["s"] = m_s;
    json["c"] = m_c;
    json["S"] = m_S;
    json["o"] = m_o;
    json["f"] = m_f;
    json["q"] = std::to_string(m_q);
    json["p"] = std::to_string(m_p);
    json["ap"] = std::to_string(m_ap);
    json["sp"] = std::to_string(m_sp);
    json["x"] = m_x;
    json["X"] = m_X;
    json["i"] = m_i;
    json["l"] = std::to_string(m_l);
    json["z"] = std::to_string(m_z);
    json["L"] = std::to_string(m_L);
    json["T"] = m_T;
    json["t"] = m_t;
    json["b"] = std::to_string(m_b);
    json["a"] = std::to_string(m_a);
    json["m"] = m_m;
    json["R"] = m_R;
    json["wt"] = m_wt;
    json["ot"] = m_ot;
    json["ps"] = m_ps;
    json["cp"] = m_cp;
    json["AP"] = std::to_string(m_AP);
    json["cr"] = std::to_string(m_cr);
    json["rp"] = std::to_string(m_rp);

    return json;
}

void EventOrderUpdate::fromJson(const nlohmann::json &json) {
    Event::fromJson(json);

    readValue<std::string>(json, "s", m_s);
    readValue<std::string>(json, "c", m_c);
    readMagicEnum<Side>(json, "S", m_S);
    readMagicEnum<OrderType>(json, "o", m_o);
    readMagicEnum<TimeInForce>(json, "f", m_f);
    m_q = readStringAsDouble(json, "q");
    m_p = readStringAsDouble(json, "p");
    m_ap = readStringAsDouble(json, "ap");
    m_sp = readStringAsDouble(json, "sp");
    readMagicEnum<ExecutionType>(json, "x", m_x);
    readMagicEnum<OrderStatus>(json, "X", m_X);
    readValue<std::int64_t>(json, "i", m_i);
    m_l = readStringAsDouble(json, "l");
    m_z = readStringAsDouble(json, "z");
    m_L = readStringAsDouble(json, "L");
    readValue<std::int64_t>(json, "T", m_T);
    readValue<std::int64_t>(json, "t", m_t);
    m_b = readStringAsDouble(json, "b");
    m_a = readStringAsDouble(json, "a");
    readValue<bool>(json, "m", m_m);
    readValue<bool>(json, "R", m_R);
    readMagicEnum<WorkingType>(json, "wt", m_wt);
    readMagicEnum<OrderType>(json, "ot", m_ot);
    readMagicEnum<PositionSide>(json, "ps", m_ps);
    readValue<bool>(json, "cp", m_cp);
    m_AP = readStringAsDouble(json, "AP");
    m_cr = readStringAsDouble(json, "cr");
    m_rp = readStringAsDouble(json, "rp");
}

nlohmann::json EventUserData::toJson() const {
    throw std::runtime_error("Unimplemented: EventUserData::toJson()");
}

void EventUserData::fromJson(const nlohmann::json &json) {
    Event ev;
    ev.fromJson(json);

    switch (ev.m_e) {
        case EventType::ORDER_TRADE_UPDATE: {
            if (const auto it = json.find("o"); it != json.end()) {
                EventOrderUpdate evOrderUpdate;
                evOrderUpdate.fromJson(*it);
                eventData = evOrderUpdate;
            }
            break;
        }
        case EventType::ACCOUNT_CONFIG_UPDATE: {
            if (const auto it = json.find("a"); it != json.end()) {
                EventBalanceUpdate evBalanceUpdate;
                evBalanceUpdate.fromJson(*it);
                eventData = evBalanceUpdate;
            }
            break;
        }
        case EventType::ACCOUNT_UPDATE: {
            if (const auto it = json.find("ac"); it != json.end()) {
                EventAccountUpdate evAccountUpdate;
                evAccountUpdate.fromJson(*it);
                eventData = evAccountUpdate;
            }
            break;
        }
        case EventType::listenKeyExpired: {
            eventData = ev;
            break;
        }
        default: eventData = Event{};
    }
}

nlohmann::json EventAggregatedTrade::toJson() const {
    throw std::runtime_error("Unimplemented: EventAggregatedTrade::toJson()");
}

void EventAggregatedTrade::fromJson(const nlohmann::json &json) {
    readValue<std::string>(json, "s", m_s);
    readValue<std::int64_t>(json, "a", m_a);
    m_p = readStringAsDouble(json, "p");
    m_q = readStringAsDouble(json, "q");
    readValue<std::int64_t>(json, "f", m_f);
    readValue<std::int64_t>(json, "l", m_l);
    readValue<std::int64_t>(json, "T", m_T);
    readValue<bool>(json, "m", m_m);
}

[[nodiscard]] nlohmann::json EventCandlestick::Candlestick::toJson() const {
    nlohmann::json json;
    json["t"] = m_t;
    json["T"] = m_T;
    json["s"] = m_s;
    json["f"] = m_f;
    json["L"] = m_L;

    auto intervalString = std::string(magic_enum::enum_name(m_i));
    intervalString.erase(0, 1);
    json["i"] = intervalString;

    json["o"] = std::to_string(m_o);
    json["h"] = std::to_string(m_h);
    json["l"] = std::to_string(m_l);
    json["c"] = std::to_string(m_c);
    json["v"] = std::to_string(m_v);
    json["n"] = m_n;
    json["x"] = m_x;
    json["q"] = m_q;
    json["V"] = m_V;
    json["Q"] = m_Q;
    return json;
}

void EventCandlestick::Candlestick::fromJson(const nlohmann::json &json) {
    readValue<std::int64_t>(json, "t", m_t);
    readValue<std::int64_t>(json, "T", m_T);
    readValue<std::string>(json, "s", m_s);
    readValue<std::int64_t>(json, "f", m_f);
    readValue<std::int64_t>(json, "L", m_L);

    std::string interval;
    readValue<std::string>(json, "i", interval);
    interval.insert(0, "_");

    if (const auto m_i_val = magic_enum::enum_cast<CandleInterval>(interval, magic_enum::case_insensitive); m_i_val.
        has_value()) {
        m_i = m_i_val.value();
    }

    m_o = readStringAsDouble(json, "o");
    m_h = readStringAsDouble(json, "h");
    m_l = readStringAsDouble(json, "l");
    m_c = readStringAsDouble(json, "c");
    m_v = readStringAsDouble(json, "v");
    readValue<std::int64_t>(json, "n", m_n);
    readValue<bool>(json, "x", m_x);
    m_q = readStringAsDouble(json, "q");
    m_V = readStringAsDouble(json, "V");
    m_Q = readStringAsDouble(json, "Q");
}

[[nodiscard]] nlohmann::json EventCandlestick::toJson() const {
    nlohmann::json json;
    json["s"] = m_s;
    json["k"] = m_k.toJson();
    return json;
}

void EventCandlestick::fromJson(const nlohmann::json &json) {
    readValue<std::string>(json, "s", m_s);
    m_k.fromJson(json["k"]);
}
}
