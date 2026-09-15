#pragma once

#include <chrono>
#include <string>

class RedisConnection;

class RedisRateLimiter {
    public:
        explicit RedisRateLimiter(RedisConnection& connection);
        bool IsAllowed(const std::string& key,int max_requests,std::chrono::seconds window);

    private:
        RedisConnection& connection_;
};

