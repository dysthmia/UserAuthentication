#pragma once
#include <chrono>
#include <string>
#include "Enums/UserRole.h"
#include "ValueObjects/UserId/UserId.h"
#include "ValueObjects/PersonalData/PersonalData.h"

using TimePoint = std::chrono::system_clock::time_point;

class User {
    public:
    static User CreateDefaultUser(
        std::string name,
        std::string username,
        std::string email,
        std::string password_hash
    );

    static User CreateAdminUser(
        std::string name,
        std::string username,
        std::string email,
        std::string password_hash
    );

    static User Restore(
        UserId user_id,
        PersonalData personal_data,
        UserRole user_role,
        std::string password_hash,
        TimePoint created_at,
        TimePoint updated_at
    );
        
    private:
        explicit User(
            UserId user_id,
            PersonalData personal_data,
            UserRole user_role,
            std::string password_hash,
            TimePoint created_at,
            TimePoint updated_at
        );

        UserId user_id_;
        PersonalData personal_data_;
        UserRole user_role_;

        std::string password_hash_;

        TimePoint created_at_;
        TimePoint updated_at_;
};