#include "User.h"
#include <format>

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

std::string User::GetStringRole() const {
    switch (user_role_)
    {
    case UserRole::User: return "Default User";
    case UserRole::Admin : return "Admin";
    default:
        return "UNKNOW";
    }
}

const std::string User::GetAllInformation() const {
    std::string user_id       = "User ID: " + user_id_.GetValue();
    std::string user_role     ="User Role: " +  GetStringRole();      
    std::string personal_data = "Name: "+ personal_data_.GetName() + "; Username: "  
                              + personal_data_.GetUsername() + "; Email: " 
                              + personal_data_.GetEmail();
    std::string password_hash ="Password hash: " +  password_hash_;
    std::string timing ="Created at: " + std::format("{:%Y-%m-%d %H:%M:%S}", created_at_) 
                       + "; Updated at: " +
                       std::format("{:%Y-%m-%d %H:%M:%S}", updated_at_) ;

    const std::string  answer = user_id + "\n" + user_role + "\n" + personal_data + "\n"
         + password_hash + "\n" + timing;

    return answer;
}