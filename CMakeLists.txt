cmake_minimum_required(VERSION 3.21)
project(UserAuthentication LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(libpqxx CONFIG REQUIRED)
find_package(redis++ CONFIG REQUIRED)
find_package(jwt-cpp CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(unofficial-argon2 CONFIG REQUIRED)
find_package(stduuid CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

add_subdirectory(src/Core)
add_subdirectory(src/Infrastructure)
add_subdirectory(src/Api)

add_executable(${PROJECT_NAME} src/main.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE
    core
    infrastructure
    api
    libpqxx::pqxx
    redis++::redis++_static
    jwt-cpp::jwt-cpp
    OpenSSL::SSL OpenSSL::Crypto
    unofficial::argon2::libargon2
    stduuid
    nlohmann_json::nlohmann_json
)