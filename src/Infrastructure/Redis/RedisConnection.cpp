#include "RedisConnection.h"

#include <hiredis/hiredis.h>
#include <stdexcept>
#include <string>
#include <utility>

struct RedisConnection::Impl {
    std::string host;
    int port;
    redisContext* context = nullptr;
};

namespace {
    // RAII wrapper around hiredis's redisReply so every reply is freed on exit.
    class RedisReply {
        public:
            explicit RedisReply(redisReply* reply) : reply_(reply) {}
            ~RedisReply() {
                if (reply_ != nullptr) {
                    freeReplyObject(reply_);
                }
            }
            RedisReply(const RedisReply&) = delete;
            RedisReply& operator=(const RedisReply&) = delete;

            redisReply* get() const noexcept { return reply_; }

        private:
            redisReply* reply_;
    };

    // Validates a reply returned by redisCommand. Throws on transport-level or
    // command-level (Redis) errors.
    RedisReply CheckReply(redisContext* context, redisReply* reply) {
        if (reply == nullptr) {
            throw std::runtime_error(
                std::string("Redis command failed: ") + context->errstr
            );
        }

        RedisReply result(reply);

        if (result.get()->type == REDIS_REPLY_ERROR) {
            std::string error =
                result.get()->str != nullptr
                    ? result.get()->str
                    : "unknown error";

            throw std::runtime_error("Redis command failed: " + error);
        }

        return result;
    }
} 

RedisConnection::RedisConnection(std::string host, int port)
    : impl_(std::make_unique<Impl>())
{
    impl_->host = std::move(host);
    impl_->port = port;
}

RedisConnection::~RedisConnection() {
    Disconnect();
}

RedisConnection::RedisConnection(RedisConnection&& other) noexcept
    : impl_(std::move(other.impl_))
{
}

RedisConnection& RedisConnection::operator=(RedisConnection&& other) noexcept {
    if (this != &other) {
        Disconnect();
        impl_ = std::move(other.impl_);
    }
    return *this;
}

void RedisConnection::Connect() {
    if (IsConnected()) {
        return;
    }

    Disconnect();

    impl_->context = redisConnect(impl_->host.c_str(), impl_->port);

    if (impl_->context == nullptr) {
        throw std::runtime_error("Redis: failed to allocate connection");
    }

    if (impl_->context->err != 0) {
        std::string error = impl_->context->errstr;
        redisFree(impl_->context);
        impl_->context = nullptr;
        throw std::runtime_error("Redis connection failed: " + error);
    }
}

void RedisConnection::Disconnect() {
    if (impl_ && impl_->context != nullptr) {
        redisFree(impl_->context);
        impl_->context = nullptr;
    }
}

bool RedisConnection::IsConnected() const {
    return impl_ && impl_->context != nullptr && impl_->context->err == 0;
}

redisContext* RedisConnection::Handle() const noexcept {
    if (!impl_) {
        return nullptr;
    }
    return impl_->context;
}

redisContext* RedisConnection::Connection() {
    if (!IsConnected()) {
        Connect();
    }
    return impl_->context;
}


std::optional<std::string> RedisConnection::Get(const std::string& key) {
    redisContext* context = Connection();

    redisReply* reply = redisCommand(
        context,
        "GET %b",
        key.data(),
        key.size()
    );

    RedisReply result = CheckReply(context, reply);

    if (result.get()->type == REDIS_REPLY_NIL) {
        return std::nullopt;
    }

    return std::string(result.get()->str, result.get()->len);
}

void RedisConnection::Set(const std::string& key, const std::string& value) {
    redisContext* context = Connection();

    redisReply* reply = redisCommand(
        context,
        "SET %b %b",
        key.data(), key.size(),
        value.data(), value.size()
    );

    CheckReply(context, reply);
}

void RedisConnection::SetEx(
        const std::string& key,
        const std::string& value,
        std::chrono::seconds ttl
    )
{
    redisContext* context = Connection();

    redisReply* reply = redisCommand(
        context,
        "SET %b %b EX %lld",
        key.data(), key.size(),
        value.data(), value.size(),
        static_cast<long long>(ttl.count())
    );

    CheckReply(context, reply);
}

void RedisConnection::Delete(const std::string& key) {
    redisContext* context = Connection();

    redisReply* reply = redisCommand(
        context,
        "DEL %b",
        key.data(),
        key.size()
    );

    CheckReply(context, reply);
}

bool RedisConnection::Exists(const std::string& key) {
    redisContext* context = Connection();

    redisReply* reply = redisCommand(
        context,
        "EXISTS %b",
        key.data(),
        key.size()
    );

    RedisReply result = CheckReply(context, reply);
    return result.get()->integer == 1;
}

void RedisConnection::Expire(const std::string& key, std::chrono::seconds ttl) {
    redisContext* context = Connection();

    redisReply* reply = redisCommand(
        context,
        "EXPIRE %b %lld",
        key.data(), key.size(),
        static_cast<long long>(ttl.count())
    );

    CheckReply(context, reply);
}

long long RedisConnection::Increment(const std::string& key) {
    redisContext* context = Connection();

    redisReply* reply = redisCommand(
        context,
        "INCR %b",
        key.data(),
        key.size()
    );

    RedisReply result = CheckReply(context, reply);
    return result.get()->integer;
}

