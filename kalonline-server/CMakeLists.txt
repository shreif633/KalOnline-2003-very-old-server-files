cmake_minimum_required(VERSION 3.20)
project(KalOnlineServer VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Find system packages
find_package(Threads REQUIRED)
find_package(PostgreSQL REQUIRED)
find_package(yaml-cpp REQUIRED)
find_package(Boost REQUIRED COMPONENTS system)
find_package(fmt REQUIRED)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${PostgreSQL_INCLUDE_DIRS}
    ${YAML_CPP_INCLUDE_DIR}
    ${Boost_INCLUDE_DIRS}
)

# Common sources
set(COMMON_SOURCES
    src/common/logger/Logger.cpp
    src/common/config/Config.cpp
    src/common/crypto/Crypto.cpp
    src/common/network/Network.cpp
    src/common/database/Database.cpp
    src/common/utils/Utils.cpp
)

# Auth Server
add_executable(kal-auth
    src/auth/main.cpp
    src/auth/AuthServer.cpp
    src/auth/core/AuthManager.cpp
    ${COMMON_SOURCES}
)
target_link_libraries(kal-auth
    Threads::Threads
    ${PostgreSQL_LIBRARIES}
    pqxx
    yaml-cpp
    Boost::system
    fmt::fmt
)

# DB Server
add_executable(kal-db
    src/db/main.cpp
    src/db/DBServer.cpp
    src/db/core/QueryRouter.cpp
    ${COMMON_SOURCES}
)
target_link_libraries(kal-db
    Threads::Threads
    ${PostgreSQL_LIBRARIES}
    pqxx
    yaml-cpp
    Boost::system
    fmt::fmt
)

# Main Game Server
add_executable(kal-main
    src/main/main.cpp
    src/main/GameServer.cpp
    src/main/WorldManager.cpp
    src/main/EntityManager.cpp
    src/main/CombatSystem.cpp
    src/main/SkillSystem.cpp
    src/main/ItemSystem.cpp
    src/main/SocialSystem.cpp
    src/main/QuestSystem.cpp
    src/main/MainServer.cpp
    src/main/system/SpawnManager.cpp
    src/main/system/MonsterAI.cpp
    src/main/network/PacketHandler.cpp
    src/main/core/Entities.cpp
    src/assets/KsmParser.cpp
    src/assets/AssetLoader.cpp
    ${COMMON_SOURCES}
)
target_link_libraries(kal-main
    Threads::Threads
    ${PostgreSQL_LIBRARIES}
    yaml-cpp
    Boost::system
    fmt::fmt
)
