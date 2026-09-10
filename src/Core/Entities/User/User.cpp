#include "User.h"

User::User (
        UserId user_id,
        PersonalData personal_data,
        UserRole user_role,
        std::string password_hash,
        TimePoint created_at,
        TimePoint updated_at
    )
    :
        user_id_(std::move(user_id)),
        personal_data_(std::move(personal_data)),
        user_role_(std::move(user_role)),
        password_hash_(std::move(password_hash)),
        created_at_(std::move(created_at)),
        updated_at_(std::move(updated_at))
{
}

User User::CreateDefaultUser(
        std::string name,
        std::string username,
        std::string email,
        std::string password_hash
    )
{
    PersonalData personal_data = PersonalData::Create(name,username,email);

    return User (
        UserId::Create(),
        personal_data,
        UserRole::User, 
        password_hash,
        TimePoint::clock::now(),
        TimePoint::clock::now()
    );
}

User User::CreateAdminUser (
        std::string name,
        std::string username,
        std::string email,
        std::string password_hash
    )
{
    PersonalData personal_data = PersonalData::Create(name,username,email);

    return User (
        UserId::Create(),
        personal_data,
        UserRole::Admin, 
        password_hash,
        TimePoint::clock::now(),
        TimePoint::clock::now()
    );
}

User User::Restore(
        UserId user_id,
        PersonalData personal_data,
        UserRole user_role,
        std::string password_hash,
        TimePoint created_at,
        TimePoint updated_at
    )
{
    return User(
        std::move(user_id),
        std::move(personal_data),
        user_role,
        std::move(password_hash),
        created_at,
        updated_at
    );
}