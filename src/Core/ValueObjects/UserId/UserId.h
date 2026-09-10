#pragma once

#include <string>

class UserId {
    public:
        static UserId Create();
        static UserId Restore(const std::string& value);
        const std::string& GetValue() const;

    private:
        std::string value_;
        explicit UserId(std::string value);
};