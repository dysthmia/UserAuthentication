#include "RedisSessionStore.h"

#include <nlohmann/json.hpp>

#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

#include "Redis/RedisConnection.h"

namespace {

    using TimePoint = std::chrono::system_clock::time_point;

    constexpr const char* SessionKeyPrefix = "auth:session:";
    constexpr const char* UserSessionsKeyPrefix = "auth:user_sessions:";

    std::string SessionKey(const std::string& session_id) {
        return SessionKeyPrefix + session_id;
    }

    std::string UserSessionsKey(const std::string& user_id) {
        return UserSessionsKeyPrefix + user_id;
    }

    // ISO 8601 UTC, e.g. "2026-09-10T20:00:00Z".
    std::string TimePointToIso8601(TimePoint tp) {
        std::time_t t = std::chrono::system_clock::to_time_t(tp);
        std::tm utc{};
        gmtime_r(&t, &utc);
        char buffer[32];
        std::strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%SZ", &utc);
        return std::string(buffer);
    }

    TimePoint Iso8601ToTimePoint(const std::string& text) {
        std::tm utc{};
        std::istringstream stream(text);
        stream >> std::get_time(&utc, "%Y-%m-%dT%H:%M:%SZ");
        if (stream.fail()) {
            throw std::runtime_error(
                "RedisSessionStore: failed to parse timestamp '" + text + "'"
            );
        }
        std::time_t t = timegm(&utc);
        return std::chrono::system_clock::from_time_t(t);
    }

    std::string SerializeSession(const Session& session) {
        nlohmann::json document;
        document["session_id"] = session.Id().Value();
        document["user_id"] = session.UserIdValue().GetValue();
        document["refresh_token_hash"] = session.RefreshTokenHash();
        document["created_at"] = TimePointToIso8601(session.CreatedAt());
        document["expires_at"] = TimePointToIso8601(session.ExpiresAt());
        document["ip_address"] = session.IpAddress();
        document["user_agent"] = session.UserAgent();
        return document.dump();
    }

    Session DeserializeSession(const std::string& text) {
        try {
            nlohmann::json document = nlohmann::json::parse(text);

            SessionId id(
                document.at("session_id").get<std::string>()
            );

            UserId user_id =
                UserId::Restore(
                    document.at("user_id").get<std::string>()
                );

            return Session::Restore(
                std::move(id),
                std::move(user_id),
                document.at("refresh_token_hash").get<std::string>(),
                Iso8601ToTimePoint(
                    document.at("created_at").get<std::string>()
                ),
                Iso8601ToTimePoint(
                    document.at("expires_at").get<std::string>()
                ),
                document.at("ip_address").get<std::string>(),
                document.at("user_agent").get<std::string>()
            );
        }
        catch (const nlohmann::json::exception&) {
            throw std::runtime_error(
                "RedisSessionStore: malformed session document: " +
                text
            );
        }
    }
} 

RedisSessionStore::RedisSessionStore(RedisConnection& connection)
    : connection_(connection)
{
}

void RedisSessionStore::CreateSession(const Session& session,std::chrono::seconds ttl) {
    if (ttl.count() <= 0) {
        throw std::invalid_argument(
            "RedisSessionStore: session TTL must be positive"
        );
    }

    const std::string session_key = SessionKey(session.Id().Value());

    const std::string user_sessions_key = UserSessionsKey(session.UserIdValue().GetValue());

    connection_.SetEx(session_key,SerializeSession(session),ttl);

    connection_.SAdd(user_sessions_key,session.Id().Value());
}

std::optional<Session> RedisSessionStore::GetSession(const SessionId& session_id) {
    const std::string session_key = SessionKey(session_id.Value());

    std::optional<std::string> raw = connection_.Get(session_key);
    if (!raw.has_value()) {
        return std::nullopt;
    }

    return DeserializeSession(*raw);
}

void RedisSessionStore::DeleteSession(const SessionId& session_id) {
    const std::string session_key = SessionKey(session_id.Value());

    std::optional<Session> session = GetSession(session_id);

    if (!session.has_value()) {
        return;
    }

    const std::string user_sessions_key = UserSessionsKey(session->UserIdValue().GetValue());

    connection_.Delete(session_key);
    connection_.SRem(user_sessions_key,session_id.Value());
}

void RedisSessionStore::ReplaceRefreshToken(const SessionId& session_id,const std::string& new_hash)
{
    std::optional<Session> session = GetSession(session_id);
    if (!session.has_value()) {
        throw std::runtime_error(
            "RedisSessionStore: session not found: " + session_id.Value()
        );
    }

    const TimePoint now = std::chrono::system_clock::now();

    if (session->IsExpired(now)) {
        DeleteSession(session_id);
        return;
    }

    session->RotateRefreshToken(new_hash);

    const auto remaining =std::chrono::duration_cast<std::chrono::seconds>(
            session->ExpiresAt() - now
        );

    if (remaining.count() <= 0) {
        DeleteSession(session_id);
        return;
    }

    const std::string session_key = SessionKey(session_id.Value());
    connection_.SetEx(session_key, SerializeSession(*session), remaining);
}

std::vector<Session> RedisSessionStore::GetUserSessions(UserId user_id) {
    const std::string user_sessions_key = UserSessionsKey(user_id.GetValue());

    const std::vector<std::string> session_ids = connection_.SMembers(user_sessions_key);

    std::vector<Session> sessions;
    sessions.reserve(session_ids.size());

    for (const std::string& id : session_ids) {
        std::optional<Session> session = GetSession(SessionId(id));

        if (!session.has_value()) {
            connection_.SRem(user_sessions_key, id);
            continue;
        }

        if (session->UserIdValue().GetValue() != user_id.GetValue()) {
            connection_.SRem(user_sessions_key, id);
            continue;
        }

        sessions.push_back(std::move(*session));
    }

    return sessions;
}

void RedisSessionStore::DeleteAllUserSessions(UserId user_id) {
    const std::string user_sessions_key = UserSessionsKey(user_id.GetValue());

    const std::vector<std::string> session_ids = connection_.SMembers(user_sessions_key);

    for (const std::string& id : session_ids) {
        connection_.Delete(SessionKey(id));
    }

    connection_.Delete(user_sessions_key);
}

