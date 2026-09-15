#pragma once

#include <chrono>
#include <optional>
#include <string>
#include <vector>

#include "Entities/Session/Session.h"
#include "ValueObjects/SessionId/SessionId.h"
#include "ValueObjects/UserId/UserId.h"

class RedisConnection;

// Redis-backed session storage.
//
// Sessions are stored under:
//
//     auth:session:{session_id}          -> JSON document (the session)
//     auth:user_sessions:{user_id}       -> SET of session ids per user
//
// The TTL of a session key matches the refresh session expiration.
class RedisSessionStore {
    public:
        explicit RedisSessionStore(RedisConnection& connection);

        void CreateSession(const Session& session, std::chrono::seconds ttl);
        std::optional<Session> GetSession(const SessionId& session_id);
        void DeleteSession(const SessionId& session_id);
        void ReplaceRefreshToken(const SessionId& session_id, const std::string& new_hash);
        std::vector<Session> GetUserSessions(UserId user_id);
        void DeleteAllUserSessions(UserId user_id);

    private:
        RedisConnection& connection_;
};

