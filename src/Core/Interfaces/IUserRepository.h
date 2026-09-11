#pragma once
#include <string>
#include <vector>
#include <optional>
#include "Entities/User/User.h"
#include "ValueObjects/UserId/UserId.h"

class IUserRepository {
    public:
        virtual ~IUserRepository() = default;

        virtual std::optional<User> GetById(UserId id) = 0;
        virtual std::optional<User> GetByEmail(const std::string& email) = 0;
        virtual bool ExistsByEmail(const std::string& email) = 0;
        virtual User Create(const User& user) = 0;
        virtual void Update(const User& user) = 0;
        virtual void Delete(UserId id) = 0;
        virtual std::vector<User> GetAll() = 0;
};

