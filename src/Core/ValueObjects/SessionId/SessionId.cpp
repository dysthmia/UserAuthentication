#include "SessionId.h"
#include <stdexcept>
#include <utility>

namespace {

    constexpr int CanonicalLength = 36;
    constexpr char Version4 = '4';

    bool IsHexDigit(char c) {
        return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f');
    }
    bool IsHyphenPosition(int index) {
        return index == 8 || index == 13 || index == 18 || index == 23;
    }
    bool IsVariantNibble(char c) {
        return c == '8' || c == '9' || c == 'a' || c == 'b';
    }

    void Validate(const std::string& value) {

        if (value.empty()) {
            throw std::invalid_argument("SessionId cannot be empty");
        }

        if (value.size() != CanonicalLength) {
            throw std::invalid_argument(
                "SessionId must be a canonical UUID string of 36 characters"
            );
        }

        for (int i = 0; i < value.size(); ++i) {
            if (IsHyphenPosition(i)) {
                if (value[i] != '-') {
                    throw std::invalid_argument(
                        "SessionId must be a canonical UUID string"
                    );
                }
            } else if (!IsHexDigit(value[i])) {
                throw std::invalid_argument(
                    "SessionId must be a canonical UUID string"
                );
            }
        }

        if (value[14] != Version4) {
            throw std::invalid_argument("SessionId must be a UUID version 4");
        }

        if (!IsVariantNibble(value[19])) {
            throw std::invalid_argument(
                "SessionId must be a RFC 4122 variant UUID"
            );
        }
    }
}

SessionId::SessionId(std::string value)
    : value_(std::move(value))
{
    Validate(value_);
}

const std::string& SessionId::Value() const noexcept {
    return value_;
}

bool SessionId::operator==(const SessionId& other) const noexcept {
    return value_ == other.value_;
}

