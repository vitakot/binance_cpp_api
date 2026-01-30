/**
Binance Data Models

Licensed under the MIT License <http://opensource.org/licenses/MIT>.
SPDX-License-Identifier: MIT
Copyright (c) 2022 Vitezslav Kot <vitezslav.kot@gmail.com>.
*/

#include "vk/binance/binance_event_models.h"
#include "vk/utils/utils.h"
#include "vk/utils/json_utils.h"

namespace vk::binance::futures {
nlohmann::json Event::toJson() const {
    nlohmann::json json;
    json["e"] = e;
    json["E"] = E;
    return json;
}

void Event::fromJson(const nlohmann::json &json) {
    readMagicEnum<EventType>(json, "e", e);
    readValue<std::int64_t>(json, "E", E);
}

nlohmann::json EventTickPrice::toJson() const {
    nlohmann::json json = Event::toJson();
    json["u"] = u;
    json["s"] = s;
    json["b"] = std::to_string(b);
    json["B"] = std::to_string(B);
    json["a"] = std::to_string(a);
    json["A"] = std::to_string(A);
    return json;
}

void EventTickPrice::fromJson(const nlohmann::json &json) {
    Event::fromJson(json);

    readValue<std::int64_t>(json, "u", u);
    readValue<std::int64_t>(json, "T", T);
    readValue<std::string>(json, "s", s);

    b = readStringAsDouble(json, "b");
    B = readStringAsDouble(json, "B");
    a = readStringAsDouble(json, "a");
    A = readStringAsDouble(json, "A");
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
    json["s"] = s;
    json["c"] = c;
    json["S"] = S;
    json["o"] = o;
    json["f"] = f;
    json["q"] = std::to_string(q);
    json["p"] = std::to_string(p);
    json["ap"] = std::to_string(ap);
    json["sp"] = std::to_string(sp);
    json["x"] = x;
    json["X"] = X;
    json["i"] = i;
    json["l"] = std::to_string(l);
    json["z"] = std::to_string(z);
    json["L"] = std::to_string(L);
    json["T"] = T;
    json["t"] = t;
    json["b"] = std::to_string(b);
    json["a"] = std::to_string(a);
    json["m"] = m;
    json["R"] = R;
    json["wt"] = wt;
    json["ot"] = ot;
    json["ps"] = ps;
    json["cp"] = cp;
    json["AP"] = std::to_string(AP);
    json["cr"] = std::to_string(cr);
    json["rp"] = std::to_string(rp);

    return json;
}

void EventOrderUpdate::fromJson(const nlohmann::json &json) {
    Event::fromJson(json);

    readValue<std::string>(json, "s", s);
    readValue<std::string>(json, "c", c);
    readMagicEnum<Side>(json, "S", S);
    readMagicEnum<OrderType>(json, "o", o);
    readMagicEnum<TimeInForce>(json, "f", f);
    q = readStringAsDouble(json, "q");
    p = readStringAsDouble(json, "p");
    ap = readStringAsDouble(json, "ap");
    sp = readStringAsDouble(json, "sp");
    readMagicEnum<ExecutionType>(json, "x", x);
    readMagicEnum<OrderStatus>(json, "X", X);
    readValue<std::int64_t>(json, "i", i);
    l = readStringAsDouble(json, "l");
    z = readStringAsDouble(json, "z");
    L = readStringAsDouble(json, "L");
    readValue<std::int64_t>(json, "T", T);
    readValue<std::int64_t>(json, "t", t);
    b = readStringAsDouble(json, "b");
    a = readStringAsDouble(json, "a");
    readValue<bool>(json, "m", m);
    readValue<bool>(json, "R", R);
    readMagicEnum<WorkingType>(json, "wt", wt);
    readMagicEnum<OrderType>(json, "ot", ot);
    readMagicEnum<PositionSide>(json, "ps", ps);
    readValue<bool>(json, "cp", cp);
    AP = readStringAsDouble(json, "AP");
    cr = readStringAsDouble(json, "cr");
    rp = readStringAsDouble(json, "rp");
}

nlohmann::json EventUserData::toJson() const {
    throw std::runtime_error("Unimplemented: EventUserData::toJson()");
}

void EventUserData::fromJson(const nlohmann::json &json) {
    Event ev;
    ev.fromJson(json);

    switch (ev.e) {
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
    readValue<std::string>(json, "s", s);
    readValue<std::int64_t>(json, "a", a);
    p = readStringAsDouble(json, "p");
    q = readStringAsDouble(json, "q");
    readValue<std::int64_t>(json, "f", f);
    readValue<std::int64_t>(json, "l", l);
    readValue<std::int64_t>(json, "T", T);
    readValue<bool>(json, "m", m);
}

[[nodiscard]] nlohmann::json EventCandlestick::Candlestick::toJson() const {
    nlohmann::json json;
    json["t"] = t;
    json["T"] = T;
    json["s"] = s;
    json["f"] = f;
    json["L"] = L;

    auto intervalString = std::string(magic_enum::enum_name(i));
    intervalString.erase(0, 1);
    json["i"] = intervalString;

    json["o"] = std::to_string(o);
    json["h"] = std::to_string(h);
    json["l"] = std::to_string(l);
    json["c"] = std::to_string(c);
    json["v"] = std::to_string(v);
    json["n"] = n;
    json["x"] = x;
    json["q"] = q;
    json["V"] = V;
    json["Q"] = Q;
    return json;
}

void EventCandlestick::Candlestick::fromJson(const nlohmann::json &json) {
    readValue<std::int64_t>(json, "t", t);
    readValue<std::int64_t>(json, "T", T);
    readValue<std::string>(json, "s", s);
    readValue<std::int64_t>(json, "f", f);
    readValue<std::int64_t>(json, "L", L);

    std::string interval;
    readValue<std::string>(json, "i", interval);
    interval.insert(0, "_");

    if (const auto m_i_val = magic_enum::enum_cast<CandleInterval>(interval, magic_enum::case_insensitive); m_i_val.
        has_value()) {
        i = m_i_val.value();
    }

    o = readStringAsDouble(json, "o");
    h = readStringAsDouble(json, "h");
    l = readStringAsDouble(json, "l");
    c = readStringAsDouble(json, "c");
    v = readStringAsDouble(json, "v");
    readValue<std::int64_t>(json, "n", n);
    readValue<bool>(json, "x", x);
    q = readStringAsDouble(json, "q");
    V = readStringAsDouble(json, "V");
    Q = readStringAsDouble(json, "Q");
}

[[nodiscard]] nlohmann::json EventCandlestick::toJson() const {
    nlohmann::json json;
    json["s"] = s;
    json["k"] = k.toJson();
    return json;
}

void EventCandlestick::fromJson(const nlohmann::json &json) {
    readValue<std::string>(json, "s", s);
    k.fromJson(json["k"]);
}
}
