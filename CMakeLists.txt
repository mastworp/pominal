cmake_minimum_required(VERSION 4.1)
project(pominal)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(POMINAL_SOURCES
    src/timer/timer.cpp
    src/ipc/socket.cpp
    src/notification/display.cpp
    src/daemon/service.cpp
    src/daemon/daemon.cpp
    src/cli/client.cpp
    src/cli/parser.cpp
)

set(POMINAL_HEADERS
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

add_executable(pominal
    main.cpp
    ${POMINAL_SOURCES}
    ${POMINAL_HEADERS}
)

target_include_directories(pominal PRIVATE ${CMAKE_SOURCE_DIR})

if(APPLE)
    target_compile_definitions(pominal PRIVATE __APPLE__)
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(pominal PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -Werror
    )
endif()

include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_library(pominal_lib STATIC
    ${POMINAL_SOURCES}
    ${POMINAL_HEADERS}
)
target_include_directories(pominal_lib PUBLIC ${CMAKE_SOURCE_DIR})

add_executable(pominal_tests
    tests/timer_test.cpp
    tests/protocol_test.cpp
    tests/command_test.cpp
)

target_link_libraries(pominal_tests PRIVATE
    pominal_lib
    GTest::gtest_main
    GTest::gmock_main
)

include(GoogleTest)
gtest_discover_tests(pominal_tests)
