#include "AppConfig.h"
#include <cstdlib>
#include <stdexcept>
#include <string>

namespace {
    
    const std::string& Require(const char* name) {
        const char* raw = std::getenv(name);
        if (raw == nullptr) {
            throw std::runtime_error(
                std::string("Missing required environment variable: ") + name
            );
        }
        static thread_local std::string storage;
        storage = raw;
        return storage;
    }

    int ParseInt(const char* name) {
        const std::string& value = Require(name);
        try {
            std::size_t parsed = 0;
            int result = std::stoi(value, &parsed);
            if (parsed != value.size()) {
                throw std::invalid_argument("trailing characters");
            }
            return result;
        } catch (const std::invalid_argument&) {
            throw std::runtime_error(
                std::string("Environment variable ") + name +
                " must be an integer, got: '" + value + "'"
            );
        } catch (const std::out_of_range&) {
            throw std::runtime_error(
                std::string("Environment variable ") + name +
                " is out of range: '" + value + "'"
            );
        }
    }

    std::chrono::seconds ParseSeconds(const char* name) {
        const std::string& value = Require(name);
        try {
            std::size_t parsed = 0;
            long long result = std::stoll(value, &parsed);
            if (parsed != value.size()) {
                throw std::invalid_argument("trailing characters");
            }
            return std::chrono::seconds{result};
        } catch (const std::invalid_argument&) {
            throw std::runtime_error(
                std::string("Environment variable ") + name +
                " must be an integer number of seconds, got: '" + value + "'"
            );
        } catch (const std::out_of_range&) {
            throw std::runtime_error(
                std::string("Environment variable ") + name +
                " is out of range: '" + value + "'"
            );
        }
    }
} 

AppConfig AppConfig::FromEnvironment() {
    AppConfig config;

    config.app_host = Require("APP_HOST");
    config.app_port = ParseInt("APP_PORT");

    config.postgres_host = Require("POSTGRES_HOST");
    config.postgres_port = ParseInt("POSTGRES_PORT");
    config.postgres_database = Require("POSTGRES_DB");
    config.postgres_user = Require("POSTGRES_USER");
    config.postgres_password = Require("POSTGRES_PASSWORD");

    config.redis_host = Require("REDIS_HOST");
    config.redis_port = ParseInt("REDIS_PORT");

    config.jwt_secret = Require("JWT_SECRET");
    config.jwt_issuer = Require("JWT_ISSUER");

    config.access_token_lifetime = ParseSeconds("ACCESS_TOKEN_LIFETIME_SECONDS");
    config.refresh_token_lifetime = ParseSeconds("REFRESH_TOKEN_LIFETIME_SECONDS");

    config.login_rate_limit = ParseInt("LOGIN_RATE_LIMIT");
    config.login_rate_window = ParseSeconds("LOGIN_RATE_WINDOW_SECONDS");

    return config;
}

