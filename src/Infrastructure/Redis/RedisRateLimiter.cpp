#include "RedisRateLimiter.h"
#include "Redis/RedisConnection.h"
#include <stdexcept>

RedisRateLimiter::RedisRateLimiter(RedisConnection& connection)
    : connection_(connection)
{
}

bool RedisRateLimiter::IsAllowed(const std::string& key,int max_requests,std::chrono::seconds window)
{
    if (max_requests <= 0) {
        throw std::invalid_argument(
            "RedisRateLimiter: max_requests must be positive"
        );
    }

    if (window.count() <= 0) {
        throw std::invalid_argument(
            "RedisRateLimiter: window must be positive"
        );
    }

    const long long count = connection_.Increment(key);

    if (count == 1) {
        connection_.Expire(key, window);
    }

    return count <= max_requests;
}