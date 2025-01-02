/**
Binance Common Stuff

Licensed under the MIT License <http://opensource.org/licenses/MIT>.
SPDX-License-Identifier: MIT
Copyright (c) 2022 Vitezslav Kot <vitezslav.kot@gmail.com>.
*/

#include "vk/binance/binance.h"

namespace vk::binance {
int64_t Binance::numberOfMsForCandleInterval(const CandleInterval candleInterval) {
    switch (candleInterval) {
        case CandleInterval::_1m:
            return 60000;
        case CandleInterval::_3m:
            return 60000 * 3;
        case CandleInterval::_5m:
            return 60000 * 5;
        case CandleInterval::_15m:
            return 60000 * 15;
        case CandleInterval::_30m:
            return 60000 * 30;
        case CandleInterval::_1h:
            return 60000 * 60;
        case CandleInterval::_2h:
            return 60000 * 120;
        case CandleInterval::_4h:
            return 60000 * 240;
        case CandleInterval::_6h:
            return 60000 * 360;
        case CandleInterval::_8h:
            return 60000 * 480;
        case CandleInterval::_12h:
            return 60000 * 720;
        case CandleInterval::_1d:
            return 86400000;
        case CandleInterval::_3d:
            return 86400000 * 3;
        case CandleInterval::_1w:
            return 86400000 * 7;
        case CandleInterval::_1M:
            return static_cast<int64_t>(86400000) * 30;
        default:
            return 0;
    }
}

bool Binance::isValidCandleResolution(const std::int32_t resolution, CandleInterval &candleInterval) {
    switch (resolution) {
        case 1:
            candleInterval = CandleInterval::_1m;
            return true;
        case 3:
            candleInterval = CandleInterval::_3m;
            return true;
        case 5:
            candleInterval = CandleInterval::_5m;
            return true;
        case 15:
            candleInterval = CandleInterval::_15m;
            return true;
        case 30:
            candleInterval = CandleInterval::_30m;
            return true;
        case 60:
            candleInterval = CandleInterval::_1h;
            return true;
        case 120:
            candleInterval = CandleInterval::_2h;
            return true;
        case 240:
            candleInterval = CandleInterval::_4h;
            return true;
        case 360:
            candleInterval = CandleInterval::_6h;
            return true;
        case 480:
            candleInterval = CandleInterval::_8h;
            return true;
        case 720:
            candleInterval = CandleInterval::_12h;
            return true;
        case 1440:
            candleInterval = CandleInterval::_1d;
            return true;
        case 4320:
            candleInterval = CandleInterval::_3d;
            return true;
        case 10080:
            candleInterval = CandleInterval::_1w;
            return true;
        default:
            return false;
    }
}
}
