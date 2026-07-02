#include "Session.hpp"
#include "Broker.hpp"
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <iostream>
#include <istream>
#include <system_error>

using asio::ip::tcp;
using json = nlohmann::json;

Session::Session(tcp::socket socket, Broker& broker)
    : socket_(std::move(socket)), broker_(broker) {
}

Session::~Session() {
    spdlog::debug("Session destroyed");
}

void Session::start() {
    spdlog::info("New session started from {}", socket_.remote_endpoint().address().to_string());
    do_read();
}

void Session::stop() {
    std::error_code ec;
    socket_.cancel(ec);
    socket_.close(ec);
}

void Session::deliver(const std::string& msg) {
    asio::post(socket_.get_executor(),
        [self = shared_from_this(), msg]() {
            bool write_in_progress = !self->write_queue_.empty();
            self->write_queue_.push(msg);
            if (!write_in_progress) {
                self->do_write();
            }
        });
}

void Session::do_read() {
    auto self(shared_from_this());
    asio::async_read_until(socket_, read_buffer_, '\n',
        [this, self](std::error_code ec, std::size_t /*length*/) {
            if (!ec) {
                std::istream is(&read_buffer_);
                std::string line;
                std::getline(is, line);
                
                try {
                    json payload = json::parse(line);
                    std::string action = payload.value("action", "");
                    
                    if (action == "subscribe") {
                        std::string room = payload.value("room", "");
                        if (!room.empty()) {
                            broker_.subscribe(room, self);
                        }
                    } else if (action == "publish") {
                        std::string room = payload.value("room", "");
                        std::string message = payload.value("message", "");
                        if (!room.empty() && !message.empty()) {
                            broker_.publish(room, message, self);
                        }
                    }
                } catch (const json::parse_error& e) {
                    spdlog::error("JSON parse error: {}", e.what());
                }

                do_read();
            } else {
                std::cout << "[warning] Session read error or closed\n";
                broker_.unsubscribe(self);
            }
        });
}

void Session::do_write() {
    auto self(shared_from_this());
    asio::async_write(socket_,
        asio::buffer(write_queue_.front()),
        [this, self](std::error_code ec, std::size_t /*length*/) {
            if (!ec) {
                write_queue_.pop();
                if (!write_queue_.empty()) {
                    do_write();
                }
            } else {
                std::cout << "[warning] Session write error\n";
                broker_.unsubscribe(self);
            }
        });
}
