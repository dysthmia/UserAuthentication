#pragma once
#include <string>

class SessionId {
    public:
        explicit SessionId(std::string value);
        const std::string& Value() const noexcept;
        bool operator==(const SessionId& other) const noexcept;

    private:
        std::string value_;
};

