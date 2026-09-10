#include "PersonalData.h"

PersonalData::PersonalData (
        std::string name, 
        std::string username, 
        std::string email
    )
    :   name_(std::move(name)), 
        username_(std::move(username)), 
        email_(std::move(email))
{
}

PersonalData PersonalData::Create (
        std::string name, 
        std::string username, 
        std::string email
    ) 
{
        Validate(name, "name");
        Validate(username, "username");
        ValidateEmail(email);
        return PersonalData(std::move(name), std::move(username), std::move(email));
}

PersonalData PersonalData::Restore (
        std::string name, 
        std::string username, 
        std::string email
    ) 
{
    return Create(std::move(name), std::move(username), std::move(email));
}

void PersonalData::Validate(const std::string& value, const std::string& field_name) {
    if (value.empty()){
        throw std::invalid_argument(
            field_name + "cannot be empty"
        );
    }
    if (value.size()>100){
        throw std::invalid_argument(
            field_name + "cannot contain more than 100 characters"
        );
    }
}

void PersonalData::ValidateEmail(const std::string& email) {
    if (email.empty()) {
        throw std::invalid_argument(
            "Email cannot be empty"
        );
    }

    const int at_position = email.find('@');
    if (at_position == std::string::npos) {
        throw std::invalid_argument("Invalid email");
    }
    if (at_position == 0 || at_position == email.size()-1) {
        throw std::invalid_argument("Invalid email");
    }
}

const std::string& PersonalData::GetName() const
{
    return name_;
}

const std::string& PersonalData::GetUsername() const
{
    return username_;
}

const std::string& PersonalData::GetEmail() const
{
    return email_;
}