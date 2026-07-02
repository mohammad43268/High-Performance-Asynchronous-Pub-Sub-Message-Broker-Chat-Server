#include "Server.hpp"
#include "Session.hpp"
#include <spdlog/spdlog.h>
#include <system_error>
#include <iostream>

using asio::ip::tcp;

Server::Server(asio::io_context& io_context, short port)
    : acceptor_(io_context) {
    
    tcp::endpoint endpoint(tcp::v4(), port);
    acceptor_.open(endpoint.protocol());
    acceptor_.set_option(asio::socket_base::reuse_address(true));
    acceptor_.bind(endpoint);
    acceptor_.listen();
    
    spdlog::info("Server initialized and listening on port {}", port);
    do_accept();
}

void Server::stop() {
    std::error_code ec;
    acceptor_.cancel(ec);
    acceptor_.close(ec);
    broker_.stop_all();
}

void Server::do_accept() {
    acceptor_.async_accept(
        [this](std::error_code ec, tcp::socket socket) {
            if (!ec) {
                spdlog::info("New connection accepted");
                std::make_shared<Session>(std::move(socket), broker_)->start();
            } else {
                std::cout << "[error] Accept error\n";
            }

            do_accept();
        });
}
