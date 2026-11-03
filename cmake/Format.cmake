include_guard(GLOBAL)

function(pominal_add_format_targets)
    find_program(CLANG_FORMAT_EXECUTABLE clang-format)
    find_program(CMAKE_FORMAT_EXECUTABLE cmake-format)
    if(NOT CLANG_FORMAT_EXECUTABLE OR NOT CMAKE_FORMAT_EXECUTABLE)
        message(STATUS "Format targets disabled: clang-format and cmake-format are both required")
        return()
    endif()

    file(GLOB_RECURSE cxx_sources CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/include/*.hpp"
         "${PROJECT_SOURCE_DIR}/src/*.cpp" "${PROJECT_SOURCE_DIR}/tests/*.cpp"
    )
    list(APPEND cxx_sources "${PROJECT_SOURCE_DIR}/main.cpp")

    file(GLOB cmake_sources CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/cmake/*.cmake")
    list(APPEND cmake_sources "${PROJECT_SOURCE_DIR}/CMakeLists.txt")

    add_custom_target(
        format
        COMMAND "${CLANG_FORMAT_EXECUTABLE}" -i ${cxx_sources}
        COMMAND "${CMAKE_FORMAT_EXECUTABLE}" -i ${cmake_sources}
        WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
        VERBATIM
    )
    add_custom_target(
        format-check
        COMMAND "${CLANG_FORMAT_EXECUTABLE}" --dry-run --Werror ${cxx_sources}
        COMMAND "${CMAKE_FORMAT_EXECUTABLE}" --check ${cmake_sources}
        WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
        VERBATIM
    )
endfunction()

pominal_add_format_targets()
