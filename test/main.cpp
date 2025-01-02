#include "vk/binance/binance_futures_rest_client.h"
#include "vk/binance/binance.h"
#include "vk/tools/json_utils.h"
#include "vk/tools/utils.h"
#include "vk/binance/binance_futures_ws_client.h"
#include "vk/binance/binance_ws_stream_manager.h"
#include <memory>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <thread>
#include <spdlog/spdlog.h>
#include <future>

using namespace vk::binance;
using namespace std::chrono_literals;

constexpr int HISTORY_LENGTH_IN_MS = 86400000; // 1 day

void logFunction(const vk::LogSeverity severity, const std::string &errmsg) {
    switch (severity) {
        case vk::LogSeverity::Info:
            spdlog::info(errmsg);
            break;
        case vk::LogSeverity::Warning:
            spdlog::warn(errmsg);
            break;
        case vk::LogSeverity::Critical:
            spdlog::critical(errmsg);
            break;
        case vk::LogSeverity::Error:
            spdlog::error(errmsg);
            break;
        case vk::LogSeverity::Debug:
            spdlog::debug(errmsg);
            break;
        case vk::LogSeverity::Trace:
            spdlog::trace(errmsg);
            break;
    }
}

std::pair<std::string, std::string> readCredentials() {
    std::filesystem::path pathToCfg{"PATH_TO_CONFIG_FILE"};
    std::ifstream ifs(pathToCfg.string());

    if (!ifs.is_open()) {
        std::cerr << "Couldn't open config file: " + pathToCfg.string();
        return {};
    }

    try {
        std::string apiKey;
        std::string apiSecret;
        std::string subAccountName;

        nlohmann::json json = nlohmann::json::parse(ifs);
        vk::readValue<std::string>(json, "ApiKey", apiKey);
        vk::readValue<std::string>(json, "ApiSecret", apiSecret);

        std::pair retVal(apiKey, apiSecret);
        return retVal;
    } catch (const std::exception &e) {
        std::cerr << e.what();
        ifs.close();
    }

    return {};
}

void testBinance() {
    auto wsClient = std::make_unique<futures::WebSocketClient>();

    const auto [fst, snd] = readCredentials();
    const auto restClient = std::make_unique<futures::RESTClient>(
        fst, snd);

    const auto nowTimestamp = std::chrono::seconds(std::time(nullptr)).count() * 1000;
    constexpr int64_t thenTimestamp = 1662725807000;

    auto candles = restClient->getHistoricalPrices("BTCUSDT", CandleInterval::_1m, thenTimestamp, nowTimestamp);
}

[[noreturn]] void testWsManager() {
    const auto [fst, snd] = readCredentials();
    auto restClient = std::make_shared<futures::RESTClient>(fst, snd);
    const std::shared_ptr wsManager = std::make_unique<futures::WSStreamManager>(
        restClient);

    wsManager->setLoggerCallback(&logFunction);
    wsManager->subscribeBookTickerStream("BTCUSDT", true);
    wsManager->subscribeBookTickerStream("ETHUSDT", true);

    while (true) {
        {
            if (const auto ret = wsManager->readEventTickPrice("BTCUSDT")) {
                std::cout << "BTC price: " << ret->m_a << std::endl;
            } else {
                std::cout << "Error" << std::endl;
            }
        } {
            if (const auto ret = wsManager->readEventTickPrice("ETHUSDT")) {
                std::cout << "ETH price: " << ret->m_a << std::endl;
            } else {
                std::cout << "Error" << std::endl;
            }
        }

        std::this_thread::sleep_for(1000ms);
        wsManager->subscribeBookTickerStream("BTCUSDT", false);
        wsManager->subscribeBookTickerStream("ETHUSDT", false);
    }
}

[[noreturn]] void testWsManagerCandles() {
    const auto [fst, snd] = readCredentials();
    auto restClient = std::make_shared<futures::RESTClient>(fst, snd);
    std::shared_ptr wsManager = std::make_unique<futures::WSStreamManager>(
        restClient);

    wsManager->setLoggerCallback(&logFunction);
    wsManager->subscribeCandlestickStream("BTCUSDT", CandleInterval::_1m, true);

    while (true) {
        {
            if (auto ret = wsManager->readEventCandlestick("BTCUSDT", CandleInterval::_1m, true)) {
                std::stringstream ss;

                std::string candleEventStartTime = vk::getDateTimeStringFromTimeStamp(ret->m_k.m_t,
                    "%Y-%m-%dT%H:%M:%S", true);
                std::string candleEventStopTime = vk::getDateTimeStringFromTimeStamp(ret->m_k.m_T,
                    "%Y-%m-%dT%H:%M:%S", true);

                ss << "Previous Candle start: " << candleEventStartTime << ", candle end: " << candleEventStopTime;


                logFunction(vk::LogSeverity::Info, ss.str());
            } else {
                std::cout << "nasrat" << std::endl;
            }
        }

        std::this_thread::sleep_for(1000ms);
    }
}

std::vector<std::string> loadAssets(const std::string &path) {
    std::vector<std::string> retVal;

    if (std::ifstream inFile(path); inFile.is_open()) {
        std::string row;
        std::getline(inFile, row);

        while (std::getline(inFile, row)) {
            if (!row.empty()) {
                if (auto records = vk::splitString(row, ','); !records.empty()) {
                    retVal.push_back(records[0]);
                }
            }
        }

        inFile.close();
    }

    return retVal;
}

void testRestCandlesDownload() {
    const auto [fst, snd] = readCredentials();
    auto restClient = std::make_shared<futures::RESTClient>(fst, snd);
    std::shared_ptr wsManager = std::make_unique<
        futures::WSStreamManager>(
        restClient);

    wsManager->setLoggerCallback(&logFunction);
    wsManager->subscribeBookTickerStream("BTCUSDT");
    wsManager->subscribeBookTickerStream("ETHUSDT");

    while (true) {
        {
            if (auto ret = wsManager->readEventTickPrice("BTCUSDT", true)) {
                std::stringstream ss;
                ss << "BTCUSDT Book ticker received: " << ret.value().toJson().dump();
                logFunction(vk::LogSeverity::Info, ss.str());
            } else {
                std::cout << "BTCUSDT Error" << std::endl;
            }
        } {
            if (auto ret = wsManager->readEventTickPrice("ETHUSDT", true)) {
                std::stringstream ss;
                ss << "ETHUSDT Book ticker received: " << ret.value().toJson().dump();
                logFunction(vk::LogSeverity::Info, ss.str());
            } else {
                std::cout << "ETHUSDT Error" << std::endl;
            }
        }

        wsManager->subscribeBookTickerStream("BTCUSDT");
        wsManager->subscribeBookTickerStream("ETHUSDT");
        std::this_thread::sleep_for(1000ms);
    }
}

[[noreturn]] void testWebsockets() {
    const auto [fst, snd] = readCredentials();
    auto restClient = std::make_shared<futures::RESTClient>(fst, snd);

    for (const auto rs = restClient->getPositionRisk("BTCUSDT"); const auto &risk: rs) {
        logFunction(vk::LogSeverity::Info, fmt::format("Position risk: {}", risk.toJson().dump()));
    }

    const std::shared_ptr wsManager = std::make_unique<
        futures::WSStreamManager>(
        restClient);

    wsManager->setLoggerCallback(&logFunction);
    wsManager->subscribeBookTickerStream("BTCUSDT");
    wsManager->subscribeBookTickerStream("ETHUSDT");
    wsManager->subscribeCandlestickStream("BTCUSDT", CandleInterval::_1m);
    wsManager->subscribeCandlestickStream("ETHUSDT", CandleInterval::_1m);

    while (true) {
        {
            if (auto ret = wsManager->readEventTickPrice("BTCUSDT", true)) {
                std::stringstream ss;
                ss << "BTCUSDT Book ticker received: " << ret.value().toJson().dump();
                logFunction(vk::LogSeverity::Info, ss.str());
            } else {
                std::cout << "BTCUSDT nasrat" << std::endl;
            }
        }

        std::this_thread::sleep_for(1000ms);
    }
}


void testRisk() {
    const auto [fst, snd] = readCredentials();
    const auto restClient = std::make_shared<futures::RESTClient>(fst, snd);

    for (const auto rs = restClient->getPositionRisk("BTCUSDT"); const auto &risk: rs) {
        logFunction(vk::LogSeverity::Info, fmt::format("Position risk: {}", risk.toJson().dump()));
    }
}

void testCandlesLimits() {
    const auto [fst, snd] = readCredentials();
    const auto restClient = std::make_shared<futures::RESTClient>(fst, snd);

    const auto nowTimestamp = std::chrono::seconds(std::time(nullptr)).count() * 1000;
    constexpr int64_t oldestBNBDate = 1420070400000;

    const auto prd = restClient->getHistoricalPrices("BTCUSDT", CandleInterval::_1m, oldestBNBDate,
                                                     nowTimestamp, 1500);
    logFunction(vk::LogSeverity::Info, fmt::format("Done, candles num: {}", prd.size()));
}

[[noreturn]] void measureRestResponses() {
    const auto [fst, snd] = readCredentials();
    const auto restClient = std::make_shared<futures::RESTClient>(fst, snd);

    using std::chrono::high_resolution_clock;
    using std::chrono::duration_cast;
    using std::chrono::duration;
    using std::chrono::milliseconds;

    double overallTime = 0.0;
    int numPass = 0;

    while (true) {
        auto t1 = high_resolution_clock::now();
        auto pr = restClient->getPositionRisk("BTCUSDT");
        auto t2 = high_resolution_clock::now();

        duration<double, std::milli> ms_double = t2 - t1;
        logFunction(vk::LogSeverity::Info, fmt::format("Get Position risk request time: {} ms", ms_double.count()));
        overallTime += ms_double.count();

        t1 = high_resolution_clock::now();
        auto ex = restClient->getExchangeInfo(true);
        t2 = high_resolution_clock::now();

        ms_double = t2 - t1;
        logFunction(vk::LogSeverity::Info, fmt::format("Get Exchange request time: {} ms", ms_double.count()));
        overallTime += ms_double.count();

        t1 = high_resolution_clock::now();
        const auto account = restClient->getAccountInfo();
        t2 = high_resolution_clock::now();

        ms_double = t2 - t1;
        logFunction(vk::LogSeverity::Info, fmt::format("Get Account info request time: {} ms\n", ms_double.count()));
        overallTime += ms_double.count();
        numPass++;

        double timePerResponse = overallTime / (numPass * 3);
        logFunction(vk::LogSeverity::Info, fmt::format("Average time per response: {} ms\n", timePerResponse));

        std::this_thread::sleep_for(2s);
    }
}

void setLeverage() {
    const auto [fst, snd] = readCredentials();
    const auto restClient = std::make_shared<futures::RESTClient>(fst, snd);

    try {
        auto resp = restClient->changeInitialLeverage("ETHUSDT", 5);
    } catch (std::exception &e) {
        logFunction(vk::LogSeverity::Info, fmt::format("Exception: {}", e.what()));
    }
}

void testFR() {
    const auto [fst, snd] = readCredentials();
    const auto restClient = std::make_shared<futures::RESTClient>(fst, snd);

    try {
        const auto nowTimestamp = std::chrono::seconds(std::time(nullptr)).count() * 1000;
        constexpr int64_t oldestBNBDate = 1420070400000;

        const auto data = restClient->getFundingRates("APTUSDT", oldestBNBDate, nowTimestamp, 1000);

        for (auto i = 0; i < data.size() - 1; i++) {
            if (const auto diff = (data[i + 1].m_fundingTime - data[i].m_fundingTime) / 1000;
                diff > 28810 || diff < 28790) {
            }

            std::cout << data[i].m_fundingTime << std::endl;
        }
    } catch (std::exception &e) {
        logFunction(vk::LogSeverity::Info, fmt::format("Exception: {}", e.what()));
    }
}

void testBookDepthStream() {
    const auto wsClient = std::make_unique<futures::WebSocketClient>();
    wsClient->setLoggerCallback(&logFunction);

    wsClient->partialBookDepthStream("BTCUSDT", 5, [&](const nlohmann::json &msg) {
        logFunction(vk::LogSeverity::Info, fmt::format("Msg: {}", msg.dump()));
    });

    wsClient->run();
}

void testBuySellVolume() {
    const auto [fst, snd] = readCredentials();
    const auto restClient = std::make_shared<futures::RESTClient>(fst, snd);

    try {
        const auto nowTimestamp = std::chrono::seconds(std::time(nullptr)).count() * 1000;
        const auto data = restClient->getBuySellVolume("ETHUSDT", futures::StatisticsPeriod::_1h,
                                                       nowTimestamp);

        for (auto i = 0; i < data.size() - 1; i++) {
            if (const auto diff = data[i + 1].m_timestamp - data[i].m_timestamp; diff != 60 * 60 * 1000) {
            }

            std::cout << data[i].m_timestamp << std::endl;
        }
    } catch (std::exception &e) {
        logFunction(vk::LogSeverity::Info, fmt::format("Exception: {}", e.what()));
    }
}

void testAccountBalance() {
    try {
        const auto [fst, snd] = readCredentials();
        const auto restClient = std::make_shared<futures::RESTClient>(fst, snd);

        for (auto balances = restClient->getAccountBalances(); const auto &balance: balances) {
            logFunction(vk::LogSeverity::Info, fmt::format("Balance: {}", balance.m_balance));
        }
    } catch (std::exception &e) {
        logFunction(vk::LogSeverity::Info, fmt::format("Exception: {}", e.what()));
    }
}

int main() {
    testBinance();
    testWsManagerCandles();
    testCandlesLimits();
    testRisk();
    measureRestResponses();
    testFR();
    testAccountBalance();
    return getchar();
}
