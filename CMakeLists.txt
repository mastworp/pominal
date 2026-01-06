cmake_minimum_required(VERSION 3.20)
project(pominal
    VERSION 1.0.0
    DESCRIPTION "Terminal-based Pomodoro timer service"
    LANGUAGES CXX
)

# Options
option(POMINAL_BUILD_TESTS "Build the test suite" ON)

# Global settings
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Core library - compiled once, used by both executable and tests
add_library(pominal_lib STATIC)

target_sources(pominal_lib
    PRIVATE
        src/timer/timer.cpp
        src/ipc/socket.cpp
        src/notification/display.cpp
        src/daemon/service.cpp
        src/daemon/daemon.cpp
        src/cli/client.cpp
        src/cli/parser.cpp
    PUBLIC
        FILE_SET HEADERS
        FILES
            src/utils/types.hpp
            src/timer/timer.hpp
            src/ipc/protocol.hpp
            src/ipc/socket.hpp
            src/command/command.hpp
            src/command/factory.hpp
            src/notification/sound.hpp
            src/notification/display.hpp
            src/daemon/service.hpp
            src/daemon/daemon.hpp
            src/cli/client.hpp
            src/cli/parser.hpp
)

target_include_directories(pominal_lib PUBLIC ${CMAKE_SOURCE_DIR})

if(APPLE)
    target_compile_definitions(pominal_lib PUBLIC __APPLE__)
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(pominal_lib PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -Werror
    )
endif()

# Main executable - just links to library
add_executable(pominal main.cpp)
target_link_libraries(pominal PRIVATE pominal_lib)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(pominal PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -Werror
    )
endif()

# Tests (optional)
if(POMINAL_BUILD_TESTS)
    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.14.0
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    enable_testing()

    add_executable(pominal_tests
        tests/timer_test.cpp
        tests/protocol_test.cpp
        tests/command_test.cpp
    )

    target_link_libraries(pominal_tests
        PRIVATE
            pominal_lib
            GTest::gtest_main
            GTest::gmock_main
    )

    include(GoogleTest)
    gtest_discover_tests(pominal_tests)
endif()
