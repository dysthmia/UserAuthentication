#pragma once
#include <string>

class PersonalData {
    public:
        static PersonalData Create (std::string name, std::string username, std::string email);
        static PersonalData Restore (std::string name, std::string username, std::string email);

        const std::string& GetName() const;
        const std::string& GetUsername() const;
        const std::string& GetEmail() const;

    private:
        explicit PersonalData (std::string name, std::string username, std::string email);
        static void Validate(const std::string& value,const std::string& fieldName);
        static void ValidateEmail(const std::string& email);

        std::string name_;
        std::string username_;
        std::string email_;
};