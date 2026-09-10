#include "UserId.h"
#include <random>
#include <stdexcept>

namespace 
{
    constexpr int IdLength = 16;
    constexpr char FirstCharacter = '#';
    const std::string AllowedCharacter = "abcdefghijklmnopqrstuvwxyz";
} 

UserId::UserId(std::string value)
    :value_(std::move(value))
{
}

UserId UserId::Create() {

    std::random_device rndm;
    std::mt19937 generator(rndm());
    std::uniform_int_distribution<int> distribution (
        0, AllowedCharacter.size()-1
    );

    std::string value;
    value.reserve(IdLength+1);
    value += FirstCharacter;

    for (int i=0; i<IdLength; i++) {
        value += AllowedCharacter[distribution(generator)];
    }

    return UserId(std::move(value));
}

UserId UserId::Restore(const std::string& value) {

    if (value.size()!=IdLength+1) {
        throw std::invalid_argument(
            "UserId must contain exactly 17 characters"
        );
    } 

    if (value[0] != FirstCharacter) {
        throw std::invalid_argument(
            "UserId must start with '#'"
        );
    }

    for (std::size_t i = 1; i < value.size(); ++i)
    {
        if (value[i] < 'a' || value[i] > 'z')
        {
            throw std::invalid_argument(
                "UserId must contain only lowercase English letters after '#'"
            );
        }
    }

    return UserId(value);
}

const std::string& UserId::GetValue() const
{
    return value_;
}