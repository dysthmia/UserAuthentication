#pragma once

#include <chrono>

class IClock {
    public:
        virtual ~IClock() = default;
        virtual std::chrono::system_clock::time_point Now() const = 0;
};

