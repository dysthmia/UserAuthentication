#include "UserRepository.h"
#include "PostgreSQL/PostgresConnection.h"

#include <libpq-fe.h>
#include <cctype>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace {

    class PgResult {
        public:
            explicit PgResult(PGresult* result) : result_(result) {}
            ~PgResult() {
                if (result_ != nullptr) {
                    PQclear(result_);
                }
            }
            PgResult(const PgResult&) = delete;
            PgResult& operator=(const PgResult&) = delete;

            PGresult* get() const noexcept { return result_; }

        private:
            PGresult* result_;
    };

    PgResult ExecTuples(PGconn* conn, const char* sql, const std::vector<std::string>& params) {
        std::vector<const char*> values;
        values.reserve(params.size());
        for (const auto& p : params) {
            values.push_back(p.c_str());
        }

        PGresult* result = PQexecParams(
            conn,
            sql,
            static_cast<int>(params.size()),
            nullptr,
            values.data(),
            nullptr,
            nullptr,
            0
        );

        if (result == nullptr) {
            throw std::runtime_error(
                std::string("PostgreSQL query failed: ") + PQerrorMessage(conn)
            );
        }

        if (PQresultStatus(result) != PGRES_TUPLES_OK) {
            std::string error = PQresultErrorMessage(result);
            PQclear(result);
            throw std::runtime_error("PostgreSQL query failed: " + error);
        }

        return PgResult(result);
    }

    PgResult ExecCommand(PGconn* conn, const char* sql, const std::vector<std::string>& params) {
        std::vector<const char*> values;
        values.reserve(params.size());
        for (const auto& p : params) {
            values.push_back(p.c_str());
        }

        PGresult* result = PQexecParams(
            conn,
            sql,
            static_cast<int>(params.size()),
            nullptr,
            values.data(),
            nullptr,
            nullptr,
            0
        );

        if (result == nullptr) {
            throw std::runtime_error(
                std::string("PostgreSQL query failed: ") + PQerrorMessage(conn)
            );
        }

        if (PQresultStatus(result) != PGRES_COMMAND_OK) {
            std::string error = PQresultErrorMessage(result);
            PQclear(result);
            throw std::runtime_error("PostgreSQL query failed: " + error);
        }

        return PgResult(result);
    }


    // Sets the session timezone to UTC so TIMESTAMPTZ values round-trip as UTC.
    void EnsureUtcSession(PGconn* conn) {
        PGresult* result = PQexec(conn, "SET TIME ZONE 'UTC'");
        if (result == nullptr || PQresultStatus(result) != PGRES_COMMAND_OK) {
            std::string error = (result != nullptr) ? PQresultErrorMessage(result)
                                                    : PQerrorMessage(conn);
            if (result != nullptr) {
                PQclear(result);
            }
            throw std::runtime_error("PostgreSQL: failed to set UTC timezone: " + error);
        }
        PQclear(result);
    }

    // Parse UTC "YYYY-MM-DD HH:MM:SS" back to a TimePoint.
    TimePoint TextToTimePoint(const std::string& text) {
        std::tm tm{};
        std::istringstream stream(text);
        stream >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
        if (stream.fail()) {
            throw std::runtime_error("PostgreSQL: failed to parse timestamp: '" + text + "'");
        }
        std::time_t t = timegm(&tm);
        return std::chrono::system_clock::from_time_t(t);
    }

    UserRole StringToRole(std::string value) {
        for (char& c : value) {
            c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        }
        if (value == "admin") {
            return UserRole::Admin;
        }
        if (value == "user") {
            return UserRole::User;
        }
        throw std::runtime_error(
            "PostgreSQL: unknown user role value: '" + value + "'"
        );
    }

    // Builds a User value object from a single result row (0-based row index).
    User RowToUser(PGresult* result, int row) {
        const char* id = PQgetvalue(result, row, 0);
        const char* name = PQgetvalue(result, row, 1);
        const char* username = PQgetvalue(result, row, 2);
        const char* email = PQgetvalue(result, row, 3);
        const char* password_hash = PQgetvalue(result, row, 4);
        const char* role = PQgetvalue(result, row, 5);
        const char* created_at = PQgetvalue(result, row, 6);
        const char* updated_at = PQgetvalue(result, row, 7);

        return User::Restore(
            UserId::Restore(std::string(id)),
            PersonalData::Restore(
                std::string(name),
                std::string(username),
                std::string(email)
            ),
            StringToRole(role),
            std::string(password_hash),
            TextToTimePoint(created_at),
            TextToTimePoint(updated_at)
        );
    }
} 


UserRepository::UserRepository(PostgresConnection& connection)
    : connection_(connection)
{
}

pg_conn* UserRepository::Connection() {
    if (!connection_.IsConnected()) {
        connection_.Connect();
    }
    PGconn* conn = connection_.Handle();

    if (!utc_ready_) {
        EnsureUtcSession(conn);
        utc_ready_ = true;
    }
    return conn;
}

std::optional<User> UserRepository::GetById(UserId id) {
    PGconn* conn = Connection();

    const char* sql =
        "SELECT id, name, username, email, password_hash, role, created_at, updated_at "
        "FROM users WHERE id = $1";

    std::vector<std::string> params{ id.GetValue() };

    PgResult result = ExecTuples(conn, sql, params);
    if (PQntuples(result.get()) == 0) {
        return std::nullopt;
    }
    return RowToUser(result.get(), 0);
}

std::optional<User> UserRepository::GetByEmail(const std::string& email) {
    PGconn* conn = Connection();

    const char* sql =
        "SELECT id, name, username, email, password_hash, role, created_at, updated_at "
        "FROM users WHERE email = $1";

    std::vector<std::string> params{ email };

    PgResult result = ExecTuples(conn, sql, params);
    if (PQntuples(result.get()) == 0) {
        return std::nullopt;
    }
    return RowToUser(result.get(), 0);
}

bool UserRepository::ExistsByEmail(const std::string& email) {
    PGconn* conn = Connection();

    const char* sql = "SELECT 1 FROM users WHERE email = $1";
    std::vector<std::string> params{ email };

    PgResult result = ExecTuples(conn, sql, params);
    return PQntuples(result.get()) > 0;
}

User UserRepository::Create(const User& user) {
    PGconn* conn = Connection();

    const PersonalData personal_data = user.GetUserPersonalData();

    const char* sql =
        "INSERT INTO users (id, name, username, email, password_hash, role, created_at, updated_at) "
        "VALUES ($1, $2, $3, $4, $5, $6, $7, $8)";

    std::vector<std::string> params{
        user.GetUserId().GetValue(),
        personal_data.GetName(),
        personal_data.GetUsername(),
        personal_data.GetEmail(),
        user.GetPasswordHash(),
        user.GetUserRole(),
        user.GetCreatedAt(),
        user.GetUpdatedAt(),
    };

    ExecCommand(conn, sql, params);

    return user;
}

void UserRepository::Update(const User& user) {
    PGconn* conn = Connection();

    const PersonalData personal_data = user.GetUserPersonalData();

    // created_at is immutable once persisted; only updated_at changes.
    const char* sql =
        "UPDATE users SET "
        "name = $2, username = $3, email = $4, password_hash = $5, role = $6, updated_at = $7 "
        "WHERE id = $1";

    std::vector<std::string> params{
        user.GetUserId().GetValue(),
        personal_data.GetName(),
        personal_data.GetUsername(),
        personal_data.GetEmail(),
        user.GetPasswordHash(),
        user.GetUserRole(),
        user.GetUpdatedAt(),
    };

    ExecCommand(conn, sql, params);
}

void UserRepository::Delete(UserId id) {
    PGconn* conn = Connection();

    const char* sql = "DELETE FROM users WHERE id = $1";
    std::vector<std::string> params{ id.GetValue() };

    ExecCommand(conn, sql, params);
}

std::vector<User> UserRepository::GetAll() {
    PGconn* conn = Connection();

    const char* sql =
        "SELECT id, name, username, email, password_hash, role, created_at, updated_at "
        "FROM users ORDER BY id";

    PgResult result = ExecTuples(conn, sql, {});

    std::vector<User> users;
    const int count = PQntuples(result.get());
    users.reserve(static_cast<std::size_t>(count));
    for (int i = 0; i < count; ++i) {
        users.push_back(RowToUser(result.get(), i));
    }
    return users;
}

