#pragma once

#include <asio.hpp>
#include "Broker.hpp"
#include <memory>

class Server {
public:
    Server(asio::io_context& io_context, short port);
    void stop();

private:
    void do_accept();

    asio::ip::tcp::acceptor acceptor_;
    Broker broker_;
};
