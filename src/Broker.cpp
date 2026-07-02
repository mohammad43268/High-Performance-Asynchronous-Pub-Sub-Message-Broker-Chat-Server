#include "Broker.hpp"
#include "Session.hpp"
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

void Broker::subscribe(const std::string& room, std::shared_ptr<Session> session) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Unsubscribe from previous room if exists
    auto it = session_rooms_.find(session);
    if (it != session_rooms_.end()) {
        const std::string& old_room = it->second;
        room_subscribers_[old_room].erase(session);
        if (room_subscribers_[old_room].empty()) {
            room_subscribers_.erase(old_room);
        }
    }
    
    session_rooms_[session] = room;
    room_subscribers_[room].insert(session);
    spdlog::info("Session subscribed to room: {}", room);

    // Broadcast updated user count
    json sys_msg;
    sys_msg["type"] = "user_count";
    sys_msg["room"] = room;
    sys_msg["count"] = room_subscribers_[room].size();
    std::string serialized = sys_msg.dump() + "\n";
    for (auto sub : room_subscribers_[room]) {
        sub->deliver(serialized);
    }
}

void Broker::unsubscribe(std::shared_ptr<Session> session) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = session_rooms_.find(session);
    if (it != session_rooms_.end()) {
        const std::string& room = it->second;
        room_subscribers_[room].erase(session);
        if (room_subscribers_[room].empty()) {
            room_subscribers_.erase(room);
        }
        session_rooms_.erase(it);
        spdlog::info("Session unsubscribed from room: {}", room);
        
        // Broadcast updated user count if room still exists
        auto room_it = room_subscribers_.find(room);
        if (room_it != room_subscribers_.end()) {
            json sys_msg;
            sys_msg["type"] = "user_count";
            sys_msg["room"] = room;
            sys_msg["count"] = room_it->second.size();
            std::string serialized = sys_msg.dump() + "\n";
            for (auto sub : room_it->second) {
                sub->deliver(serialized);
            }
        }
    }
}

void Broker::publish(const std::string& room, const std::string& message, std::shared_ptr<Session> sender) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = room_subscribers_.find(room);
    if (it != room_subscribers_.end()) {
        json payload;
        payload["room"] = room;
        payload["message"] = message;
        std::string serialized = payload.dump() + "\n";
        
        for (auto subscriber : it->second) {
            if (subscriber != sender) {
                subscriber->deliver(serialized);
            }
        }
        spdlog::info("Published message to room: {}", room);
    }
}

void Broker::stop_all() {
    std::lock_guard<std::mutex> lock(mutex_);
    for (const auto& [session, room] : session_rooms_) {
        session->stop();
    }
    session_rooms_.clear();
    room_subscribers_.clear();
}
