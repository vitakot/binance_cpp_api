# Binance C++ API Library

A modern C++20 library for interacting with the Binance Cryptocurrency Exchange API, supporting both Spot and Futures (Perpetual) markets. This library provides mechanisms for REST API calls and WebSocket streams.

## Features

-   **Spot & Futures Support**: Separate clients for Spot and Futures markets.
-   **REST API**: authenticates and manages requests for account info, trading, and market data.
-   **WebSocket API**: Efficient stream management for real-time market data (Book Ticker, Candlesticks, Depth, etc.).
-   **Async/Callback-based**: WebSocket streams use callbacks to handle incoming events.
-   **Thread-safe**: Designed with concurrency in mind.
-   **Logging**: Integrated with `spdlog` for flexible logging.

## Dependencies

This project relies on the following C++ libraries:

-   **[Boost](https://www.boost.org/)** (1.83+)
-   **[OpenSSL](https://www.openssl.org/)**
-   **[nlohmann_json](https://github.com/nlohmann/json)**
-   **[spdlog](https://github.com/gabime/spdlog)**
-   **[magic_enum](https://github.com/Neargye/magic_enum)**

Ensure these are installed and available in your system path or CMake prefix path.

## Integration

The project is built using CMake. To use it in your project, you can add it as a subdirectory.

### CMakeLists.txt Example

```cmake
cmake_minimum_required(VERSION 3.30)
project(my_trading_bot)

set(CMAKE_CXX_STANDARD 20)

# Add this library
add_subdirectory(path/to/binance_cpp_api)

add_executable(my_bot main.cpp)

# Link against binance_api
target_link_libraries(my_bot PRIVATE binance_api)
```

## Usage

### REST Client Example

```cpp
#include "vk/binance/binance_futures_rest_client.h"
#include <iostream>

using namespace vk::binance;

int main() {
    // Initialize with API Key and Secret
    // Use empty strings for public endpoints
    auto restClient = std::make_unique<futures::RESTClient>("YOUR_API_KEY", "YOUR_SECRET_KEY");

    // Public Request: Get Exchange Info
    auto exchangeInfo = restClient->getExchangeInfo();
    std::cout << "Server Time: " << exchangeInfo.m_serverTime << std::endl;

    // Private Request: Get Account Info
    // try-catch required for network or API errors
    try {
        auto account = restClient->getAccountInfo();
        std::cout << "Can Trade: " << account.m_canTrade << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}
```

### WebSocket Stream Example

```cpp
#include "vk/binance/binance_futures_rest_client.h"
#include "vk/binance/binance_ws_stream_manager.h"
#include <iostream>
#include <thread>

using namespace vk::binance;

// Simple logger callback
void logFunction(const vk::LogSeverity severity, const std::string &msg) {
    std::cout << "[LOG] " << msg << std::endl;
}

int main() {
    auto restClient = std::make_shared<futures::RESTClient>("", "");
    
    // Initialize WS Stream Manager
    auto wsManager = std::make_unique<futures::WSStreamManager>(restClient);
    wsManager->setLoggerCallback(&logFunction);

    // Subscribe to Book Ticker for BTCUSDT
    wsManager->subscribeBookTickerStream("BTCUSDT");

    // Main loop to process events
    while (true) {
        // Poll for events
        if (auto tick = wsManager->readEventTickPrice("BTCUSDT")) {
             std::cout << "New Ask Price: " << tick->m_a << std::endl;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    return 0;
}
```

## Building the Project

If you want to build the library and run tests directly:

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

## License

MIT License - see the [LICENSE](LICENSE) file for details.
