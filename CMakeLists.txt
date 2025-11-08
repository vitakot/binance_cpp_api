cmake_minimum_required(VERSION 3.20)
project(binance_api)

set(CMAKE_CXX_STANDARD 20)

if (MSVC)
    add_definitions(-D_WIN32_WINNT=0x0A00 /bigobj /utf-8 -DVERBOSE_LOG)
else ()
    add_definitions(-fPIC)
endif ()

if (POLICY CMP0167)
    cmake_policy(SET CMP0167 NEW)
endif ()

find_package(Boost 1.83 REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(magic_enum REQUIRED)

include_directories(include vk_cpp_common/include SYSTEM ${Boost_INCLUDE_DIR} ${OPENSSL_INCLUDE_DIR})

if (NOT TARGET vk_common)
    add_subdirectory(vk_cpp_common)
endif ()

set(HEADERS
        include/vk/binance/binance_futures_rest_client.h
        include/vk/binance/binance_spot_rest_client.h
        include/vk/binance/binance_futures_ws_client.h
        include/vk/binance/binance_models.h
        include/vk/binance/binance_event_models.h
        include/vk/binance/binance_http_session.h
        include/vk/binance/binance_futures_ws_session.h
        include/vk/binance/binance_ws_stream_manager.h
        include/vk/binance/binance.h
        include/vk/binance/binance_futures_exchange_connector.h)

set(SOURCES
        src/binance_models.cpp
        src/binance_event_models.cpp
        src/binance_futures_rest_client.cpp
        src/binance_spot_rest_client.cpp
        src/binance_futures_ws_client.cpp
        src/binance_http_session.cpp
        src/binance_futures_ws_session.cpp
        src/binance_ws_stream_manager.cpp
        src/binance.cpp
        src/binance_futures_exchange_connector.cpp)

if (MODULE_MANAGER)
    add_library(binance_api SHARED ${SOURCES} ${HEADERS})
else ()
    add_library(binance_api STATIC ${SOURCES} ${HEADERS})

    add_executable(test test/main.cpp)
    target_link_libraries(test PRIVATE spdlog::spdlog binance_api OpenSSL::Crypto)

endif ()

target_link_libraries(binance_api PRIVATE spdlog::spdlog OpenSSL::Crypto OpenSSL::SSL vk_common nlohmann_json::nlohmann_json)

if (NOT MSVC)
    target_link_libraries(binance_api atomic)
endif ()