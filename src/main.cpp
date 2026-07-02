#include <asio.hpp>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <iostream>
#include "Server.hpp"

int main() {
    try {
        // Configure spdlog formatting patterns
        spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");
        
        spdlog::info("Starting High-Performance Chat Server...");

        asio::io_context io_context;
        
        Server server(io_context, 8080);
        
        asio::signal_set signals(io_context, SIGINT, SIGTERM);
        signals.async_wait([&](const std::error_code& error, int signal_number) {
            if (!error) {
                spdlog::info("Caught termination signal. Initiating graceful shutdown sequence...");
                server.stop();
                io_context.stop();
            }
        });
        
        spdlog::info("Running event engine...");
        io_context.run();
    } catch (std::exception& e) {
        spdlog::critical("Exception: {}", e.what());
    }

    spdlog::shutdown();
    return 0;
}
