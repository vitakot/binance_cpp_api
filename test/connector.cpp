#include <vk/binance/binance_exchange_connector.h>

using namespace vk;

void testTickerInfo() {
    const auto connector = std::make_unique<BinanceFuturesExchangeConnector>();

    for (auto ticker : connector->getTickerInfo("")) {
        spdlog::info("Symbol: {}", ticker.symbol);
    }
}

int main() {
    testTickerInfo();
    return getchar();
}
