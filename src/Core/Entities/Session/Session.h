#pragma once

#include <chrono>
#include <string>

#include "ValueObjects/SessionId/SessionId.h"
#include "ValueObjects/UserId/UserId.h"

namespace {
using TimePoint = std::chrono::system_clock::time_point;
}

class Session {
    public:
        static Session Create(
            SessionId id,
            UserId user_id,
            std::string refresh_token_hash,
            TimePoint created_at,
            TimePoint expires_at,
            std::string ip_address,
            std::string user_agent
        );

        static Session Restore(
            SessionId id,
            UserId user_id,
            std::string refresh_token_hash,
            TimePoint created_at,
            TimePoint expires_at,
            std::string ip_address,
            std::string user_agent
        );

        const SessionId& Id() const noexcept;
        const UserId& UserIdValue() const noexcept;
        const std::string& RefreshTokenHash() const noexcept;
        const TimePoint& CreatedAt() const noexcept;
        const TimePoint& ExpiresAt() const noexcept;
        const std::string& IpAddress() const noexcept;
        const std::string& UserAgent() const noexcept;

        bool IsExpired(TimePoint now) const noexcept;
        void RotateRefreshToken(const std::string& new_hash);

    private:
        explicit Session(
            SessionId id,
            UserId user_id,
            std::string refresh_token_hash,
            TimePoint created_at,
            TimePoint expires_at,
            std::string ip_address,
            std::string user_agent
        );

        static void Validate(const std::string& refresh_token_hash);

        SessionId id_;
        UserId user_id_;
        std::string refresh_token_hash_;
        TimePoint created_at_;
        TimePoint expires_at_;
        std::string ip_address_;
        std::string user_agent_;
};

