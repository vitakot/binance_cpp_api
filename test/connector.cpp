#include <vk/binance/binance_futures_exchange_connector.h>

using namespace vk;

void testTickerInfo() {
    const auto connector = std::make_unique<BinanceFuturesExchangeConnector>();

    for (auto ticker : connector->getTickerInfo("")) {
        spdlog::info("Symbol: {}", ticker.symbol);
    }
}

void testFR() {
    const auto connector = std::make_unique<BinanceFuturesExchangeConnector>();
    const auto fr = connector->getFundingRates();
    spdlog::info("FR number", fr.size());
}

int main() {
    testTickerInfo();
    testFR();
    return getchar();
}
