#pragma once
#include "Interfaces/IUserRepository.h"

class PostgresConnection;
struct pg_conn;

class UserRepository final : public IUserRepository {
    public:
        explicit UserRepository(PostgresConnection& connection);

        std::optional<User> GetById(UserId id) override;
        std::optional<User> GetByEmail(const std::string& email) override;
        bool ExistsByEmail(const std::string& email) override;
        User Create(const User& user) override;
        void Update(const User& user) override;
        void Delete(UserId id) override;
        std::vector<User> GetAll() override;

    private:
        PostgresConnection& connection_;
        bool utc_ready_ = false;

        struct pg_conn* Connection();
};

