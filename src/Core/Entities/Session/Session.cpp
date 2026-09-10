#include "Session.h"

#include <stdexcept>
#include <utility>

Session::Session(
        SessionId id,
        UserId user_id,
        std::string refresh_token_hash,
        TimePoint created_at,
        TimePoint expires_at,
        std::string ip_address,
        std::string user_agent
    )
    :
        id_(std::move(id)),
        user_id_(std::move(user_id)),
        refresh_token_hash_(std::move(refresh_token_hash)),
        created_at_(created_at),
        expires_at_(expires_at),
        ip_address_(std::move(ip_address)),
        user_agent_(std::move(user_agent))
{
}

Session Session::Create(
        SessionId id,
        UserId user_id,
        std::string refresh_token_hash,
        TimePoint created_at,
        TimePoint expires_at,
        std::string ip_address,
        std::string user_agent
    )
{
    Validate(refresh_token_hash);

    return Session(
        std::move(id),
        std::move(user_id),
        std::move(refresh_token_hash),
        created_at,
        expires_at,
        std::move(ip_address),
        std::move(user_agent)
    );
}

Session Session::Restore(
        SessionId id,
        UserId user_id,
        std::string refresh_token_hash,
        TimePoint created_at,
        TimePoint expires_at,
        std::string ip_address,
        std::string user_agent
    )
{
    return Session(
        std::move(id),
        std::move(user_id),
        std::move(refresh_token_hash),
        created_at,
        expires_at,
        std::move(ip_address),
        std::move(user_agent)
    );
}

void Session::Validate(const std::string& refresh_token_hash) {
    if (refresh_token_hash.empty()) {
        throw std::invalid_argument("Refresh token hash cannot be empty");
    }
}

const SessionId& Session::Id() const noexcept {
    return id_;
}

const UserId& Session::UserIdValue() const noexcept {
    return user_id_;
}

const std::string& Session::RefreshTokenHash() const noexcept {
    return refresh_token_hash_;
}

const TimePoint& Session::CreatedAt() const noexcept {
    return created_at_;
}

const TimePoint& Session::ExpiresAt() const noexcept {
    return expires_at_;
}

const std::string& Session::IpAddress() const noexcept {
    return ip_address_;
}

const std::string& Session::UserAgent() const noexcept {
    return user_agent_;
}

bool Session::IsExpired(TimePoint now) const noexcept {
    return now >= expires_at_;
}

void Session::RotateRefreshToken(const std::string& new_hash) {
    Validate(new_hash);
    refresh_token_hash_ = new_hash;
}

