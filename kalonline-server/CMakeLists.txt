cmake_minimum_required(VERSION 3.28)
project(KalOnlineServer VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable modules support
set(CMAKE_EXPERIMENTAL_CXX_MODULE_CMAKE_API 2182bf5c-ef0d-489a-91da-49dbc309838a)
set(CMAKE_EXPERIMENTAL_CXX_MODULE_DYNDEP 1)

# Build options
option(KAL_BUILD_TESTS "Build test suite" ON)
option(KAL_BUILD_GUI "Build Qt6 management GUI" OFF)
option(KAL_USE_CONAN "Use Conan for dependency management" ON)

# Find dependencies
find_package(Threads REQUIRED)
find_package(PostgreSQL REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(yaml-cpp REQUIRED)

if(KAL_USE_CONAN)
    # Conan dependencies will be loaded via toolchain
endif()

# Common library (shared across all servers)
add_library(kal_common STATIC
    src/common/logger/Logger.cpp
    src/common/config/Config.cpp
    src/common/crypto/Crypto.cpp
    src/common/network/Network.cpp
    src/common/database/Database.cpp
    src/common/utils/Utils.cpp
)

target_include_directories(kal_common PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${PostgreSQL_INCLUDE_DIRS}
)

target_link_libraries(kal_common PUBLIC
    Threads::Threads
    ${PostgreSQL_LIBRARIES}
    nlohmann_json::nlohmann_json
    yaml-cpp::yaml-cpp
)

# Auth Server
add_executable(kal-auth
    src/auth/main.cpp
    src/auth/AuthServer.cpp
    src/auth/SessionManager.cpp
)

target_link_libraries(kal-auth PRIVATE kal_common)

# Database Server
add_executable(kal-db
    src/db/main.cpp
    src/db/DBServer.cpp
    src/db/QueryRouter.cpp
)

target_link_libraries(kal-db PRIVATE kal_common)

# Main Game Server
add_executable(kal-main
    src/main/main.cpp
    src/main/MainServer.cpp
    src/main/WorldManager.cpp
    src/main/EntityManager.cpp
    src/main/CombatSystem.cpp
    src/main/SkillSystem.cpp
    src/main/ItemSystem.cpp
    src/main/SocialSystem.cpp
    src/main/QuestSystem.cpp
)

target_link_libraries(kal-main PRIVATE kal_common)

# Tests
if(KAL_BUILD_TESTS)
    enable_testing()
    find_package(GTest REQUIRED)
    
    add_executable(kal_tests
        tests/test_crypto.cpp
        tests/test_logger.cpp
        tests/test_combat_formulas.cpp
    )
    
    target_link_libraries(kal_tests PRIVATE kal_common GTest::gtest_main)
    
    include(GoogleTest)
    gtest_discover_tests(kal_tests)
endif()

# GUI (Optional)
if(KAL_BUILD_GUI)
    find_package(Qt6 REQUIRED COMPONENTS Widgets Network Sql)
    qt_standard_project_setup()
    
    add_executable(kal-gui
        src/gui/main.cpp
        src/gui/MainWindow.cpp
        src/gui/Dashboard.cpp
        src/gui/LogViewer.cpp
        src/gui/GMCommands.cpp
    )
    
    target_link_libraries(kal-gui PRIVATE 
        kal_common
        Qt6::Widgets
        Qt6::Network
        Qt6::Sql
    )
endif()

# Installation
install(TARGETS kal-auth kal-db kal-main
    RUNTIME DESTINATION bin
)

install(FILES config/config.yaml.example
    DESTINATION etc/kalonline
)

# CPack for packaging
set(CPACK_PACKAGE_NAME "KalOnlineServer")
set(CPACK_PACKAGE_VERSION "1.0.0")
set(CPACK_PACKAGE_VENDOR "KalOnline Project")
include(CPack)
