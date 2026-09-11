#pragma once

#include <chrono>
#include <string>

class AppConfig {
    public:
        static AppConfig FromEnvironment();

        std::string app_host;
        int app_port;

        std::string postgres_host;
        int postgres_port;
        std::string postgres_database;
        std::string postgres_user;
        std::string postgres_password;

        std::string redis_host;
        int redis_port;

        std::string jwt_secret;
        std::string jwt_issuer;

        std::chrono::seconds access_token_lifetime;
        std::chrono::seconds refresh_token_lifetime;

        int login_rate_limit;
        std::chrono::seconds login_rate_window;
};

