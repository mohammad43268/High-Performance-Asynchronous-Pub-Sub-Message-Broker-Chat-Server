cmake_minimum_required(VERSION 3.14)
project(ChatServer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include(FetchContent)

# Standalone Asio
FetchContent_Declare(
    asio
    URL https://github.com/chriskohlhoff/asio/archive/refs/tags/asio-1-30-2.tar.gz
)
FetchContent_GetProperties(asio)
if(NOT asio_POPULATED)
    FetchContent_Populate(asio)
endif()

# nlohmann/json
FetchContent_Declare(
    json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
)
FetchContent_MakeAvailable(json)

# spdlog
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG        v1.12.0
)
FetchContent_MakeAvailable(spdlog)

# Executable
add_executable(chat_server
    src/main.cpp
    src/Broker.cpp
    src/Session.cpp
    src/Server.cpp
)

target_include_directories(chat_server PRIVATE include ${asio_SOURCE_DIR}/asio/include)
target_compile_definitions(chat_server PRIVATE ASIO_STANDALONE)
target_link_libraries(chat_server PRIVATE nlohmann_json::nlohmann_json spdlog::spdlog)
