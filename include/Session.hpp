#pragma once

#include <asio.hpp>
#include <memory>
#include <string>
#include <queue>

class Broker;

class Session : public std::enable_shared_from_this<Session> {
public:
    Session(asio::ip::tcp::socket socket, Broker& broker);
    ~Session();

    void start();
    void stop();
    void deliver(const std::string& msg);

private:
    void do_read();
    void do_write();

    asio::ip::tcp::socket socket_;
    Broker& broker_;
    asio::streambuf read_buffer_;
    std::queue<std::string> write_queue_;
};
