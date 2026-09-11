add_library(Core
    Entities/User/User.cpp
    Entities/Session/Session.cpp
    ValueObjects/UserId/UserId.cpp
    ValueObjects/SessionId/SessionId.cpp
    ValueObjects/PersonalData/PersonalData.cpp
)

target_include_directories(Core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

