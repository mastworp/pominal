cmake_minimum_required(VERSION 3.20)
project(pominal
    VERSION 1.0.0
    DESCRIPTION "Terminal-based Pomodoro timer service"
    LANGUAGES CXX
)

option(POMINAL_BUILD_TESTS "Build the test suite" ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(pominal_lib STATIC
    src/timer.cpp
    src/socket.cpp
    src/display.cpp
    src/service.cpp
    src/daemon.cpp
    src/client.cpp
    src/parser.cpp
)

target_include_directories(pominal_lib PUBLIC ${CMAKE_SOURCE_DIR}/include)

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

add_executable(pominal main.cpp)
target_link_libraries(pominal PRIVATE pominal_lib)

install(TARGETS pominal RUNTIME DESTINATION bin)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(pominal PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -Werror
    )
endif()

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
