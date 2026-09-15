#pragma once

#include <chrono>
#include <memory>
#include <optional>
#include <string>
#include <vector>

struct redisContext;

class RedisConnection {
    public:
        RedisConnection(std::string host, int port);
        ~RedisConnection();

        RedisConnection(RedisConnection&& other) noexcept;
        RedisConnection& operator=(RedisConnection&& other) noexcept;

        RedisConnection(const RedisConnection&) = delete;
        RedisConnection& operator=(const RedisConnection&) = delete;

        // Opens the connection. 
        void Connect();

        // Closes the connection.
        void Disconnect();

        // Returns true if the connection is currently open and usable.
        bool IsConnected() const;

        std::optional<std::string> Get(const std::string& key);
        void Set(const std::string& key, const std::string& value);
        void SetEx(const std::string& key, const std::string& value, std::chrono::seconds ttl);
        void Delete(const std::string& key);
        bool Exists(const std::string& key);
        void Expire(const std::string& key, std::chrono::seconds ttl);
        long long Increment(const std::string& key);

        // Set (collection) commands used by RedisSessionStore for the
        // user -> sessions index.
        void SAdd(const std::string& key, const std::string& member);
        void SRem(const std::string& key, const std::string& member);
        std::vector<std::string> SMembers(const std::string& key);

        // Returns the underlying hiredis context (nullable).
        redisContext* Handle() const noexcept;

    private:
        struct Impl;
        std::unique_ptr<Impl> impl_;

        redisContext* Connection();
};

