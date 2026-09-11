add_library(Infrastructure
    Config/AppConfig.cpp
    PostgreSQL/PostgresConnection.cpp
)

target_include_directories(Infrastructure
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(Infrastructure
    PUBLIC
        Domain
)

# PostgreSQL 
find_package(PostgreSQL REQUIRED)

# Redis 
find_package(hiredis REQUIRED)

# JWT 
find_package(jwt-cpp REQUIRED)

# Argon2id 
find_package(sodium REQUIRED)

# UUID 
find_package(Boost REQUIRED COMPONENTS uuid)

# JSON
find_package(nlohmann_json REQUIRED)


#Links
target_link_libraries(Infrastructure
    PUBLIC
        PostgreSQL::PostgreSQL
        hiredis::hiredis
        jwt-cpp::jwt-cpp
        sodium
        Boost::uuid
        nlohmann_json::nlohmann_json
)

