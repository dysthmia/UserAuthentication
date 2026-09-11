#pragma once
#include <memory>
#include <string>

struct pg_conn;

class PostgresConnection {
    public:
        PostgresConnection(
            std::string host,
            int port,
            std::string database,
            std::string user,
            std::string password
        );
        ~PostgresConnection();

        PostgresConnection(PostgresConnection&& other) noexcept;
        PostgresConnection& operator=(PostgresConnection&& other) noexcept;

        PostgresConnection(const PostgresConnection&) = delete;
        PostgresConnection& operator=(const PostgresConnection&) = delete;

        // Opens the connection. Throws if the connection cannot be
        // established.
        void Connect();

        // Closes the connection (safe to call multiple times).
        void Disconnect();

        // Returns true if the connection is currently open and usable.
        bool IsConnected() const;

        // Returns the underlying libpq connection handle (nullable). Intended
        // for repository implementations that run parameterized queries.
        pg_conn* Handle() const noexcept;

    private:
        struct Impl;
        std::unique_ptr<Impl> impl_;
};

