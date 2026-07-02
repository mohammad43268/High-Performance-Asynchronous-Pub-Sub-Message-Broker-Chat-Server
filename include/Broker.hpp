#pragma once

#include <unordered_map>
#include <set>
#include <string>
#include <memory>
#include <mutex>

class Session;

class Broker {
public:
    // Subscribes a session to a specific room
    void subscribe(const std::string& room, std::shared_ptr<Session> session);
    
    // Unsubscribes a session from its current room
    void unsubscribe(std::shared_ptr<Session> session);
    
    // Publishes a message to all sessions in a room, excluding the sender
    void publish(const std::string& room, const std::string& message, std::shared_ptr<Session> sender);
    
    // Gracefully stop all active sessions
    void stop_all();

private:
    std::mutex mutex_;
    std::unordered_map<std::string, std::set<std::shared_ptr<Session>>> room_subscribers_;
    std::unordered_map<std::shared_ptr<Session>, std::string> session_rooms_;
};
