#include "PostgresConnection.h"
#include <libpq-fe.h>
#include <sstream>
#include <stdexcept>
#include <utility>

struct PostgresConnection::Impl {
    std::string host;
    int port;
    std::string database;
    std::string user;
    std::string password;

    PGconn* connection = nullptr;
};

namespace {

    std::string EscapeLiteral(const std::string& value) {
        std::string escaped;
        escaped.reserve(value.size());
        for (char c : value) {
            if (c == '\\' || c == '\'') {
                escaped += '\\';
            }
            escaped += c;
        }
        return escaped;
    }

    std::string BuildConnectionString(
        const std::string& host,
        int port,
        const std::string& database,
        const std::string& user,
        const std::string& password
    ) {
        std::ostringstream conninfo;
        conninfo << "host='" << EscapeLiteral(host) << "' "
                << "port=" << port << ' '
                << "dbname='" << EscapeLiteral(database) << "' "
                << "user='" << EscapeLiteral(user) << "' "
                << "password='" << EscapeLiteral(password) << "'";
        return conninfo.str();
    }

} 

PostgresConnection::PostgresConnection(
        std::string host,
        int port,
        std::string database,
        std::string user,
        std::string password
    )
    : impl_(std::make_unique<Impl>())
{
    impl_->host = std::move(host);
    impl_->port = port;
    impl_->database = std::move(database);
    impl_->user = std::move(user);
    impl_->password = std::move(password);
}

PostgresConnection::~PostgresConnection() {
    Disconnect();
}

PostgresConnection::PostgresConnection(PostgresConnection&& other) noexcept
    : impl_(std::move(other.impl_))
{
}

PostgresConnection& PostgresConnection::operator=(PostgresConnection&& other) noexcept {
    if (this != &other) {
        Disconnect();
        impl_ = std::move(other.impl_);
    }
    return *this;
}

void PostgresConnection::Connect() {
    if (!impl_) {
        throw std::logic_error(
            "PostgreSQL: cannot connect a moved-from connection"
        );
    }
    if (IsConnected()) {
        return;
    }

    Disconnect();

    const std::string conninfo = BuildConnectionString(
        impl_->host,
        impl_->port,
        impl_->database,
        impl_->user,
        impl_->password
    );

    impl_->connection = PQconnectdb(conninfo.c_str());

    if (impl_->connection == nullptr) {
        throw std::runtime_error("PostgreSQL: failed to allocate connection");
    }

    if (PQstatus(impl_->connection) != CONNECTION_OK) {
        std::string error = PQerrorMessage(impl_->connection);
        PQfinish(impl_->connection);
        impl_->connection = nullptr;
        throw std::runtime_error("PostgreSQL connection failed: " + error);
    }
}
 
void PostgresConnection::Disconnect() {
    if (impl_ && impl_->connection != nullptr) {
        PQfinish(impl_->connection);
        impl_->connection = nullptr;
    }
}

bool PostgresConnection::IsConnected() const {
    if (!impl_ || impl_->connection == nullptr) {
        return false;
    }
    return PQstatus(impl_->connection) == CONNECTION_OK;
}

pg_conn* PostgresConnection::Handle() const noexcept {
    if (!impl_) {
        return nullptr;
    }
    return impl_->connection;
}

