include_guard(GLOBAL)

set(POMINAL_COVERAGE_MIN_LINES "40" CACHE STRING "Minimum line coverage percentage enforced by the coverage target")

function(pominal_enable_coverage)
    if(NOT POMINAL_ENABLE_COVERAGE)
        return()
    endif()
    foreach(target IN LISTS ARGN)
        target_compile_options(${target} PRIVATE --coverage -O0 -fprofile-update=atomic)
        target_link_options(${target} PUBLIC --coverage)
    endforeach()
endfunction()

function(pominal_add_coverage_target)
    if(NOT POMINAL_ENABLE_COVERAGE)
        return()
    endif()

    if(NOT CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        message(FATAL_ERROR "Coverage requires GCC or Clang, found ${CMAKE_CXX_COMPILER_ID}")
    endif()

    find_program(GCOVR_EXECUTABLE gcovr REQUIRED)

    get_filename_component(compiler_dir "${CMAKE_CXX_COMPILER}" DIRECTORY)
    string(REGEX MATCH "-[0-9]+$" compiler_suffix "${CMAKE_CXX_COMPILER}")
    if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        find_program(LLVM_COV_EXECUTABLE NAMES llvm-cov${compiler_suffix} llvm-cov HINTS "${compiler_dir}" REQUIRED)
        set(gcov_command "${LLVM_COV_EXECUTABLE} gcov")
    else()
        find_program(GCOV_EXECUTABLE NAMES gcov${compiler_suffix} gcov HINTS "${compiler_dir}" REQUIRED)
        set(gcov_command "${GCOV_EXECUTABLE}")
    endif()

    set(report_dir "${PROJECT_BINARY_DIR}/coverage-report")
    set(gcovr_args --root "${PROJECT_SOURCE_DIR}" --filter include/ --filter src/)
    list(APPEND gcovr_args --gcov-executable "${gcov_command}" --exclude-unreachable-branches --exclude-throw-branches)
    list(APPEND gcovr_args --html-title "${PROJECT_NAME} coverage" --markdown-title "${PROJECT_NAME} coverage")
    list(APPEND gcovr_args --print-summary --html-details "${report_dir}/index.html")
    list(APPEND gcovr_args --cobertura "${report_dir}/cobertura.xml" --markdown-summary "${report_dir}/summary.md")
    list(APPEND gcovr_args --fail-under-line ${POMINAL_COVERAGE_MIN_LINES})

    add_custom_target(
        coverage
        COMMAND "${CMAKE_COMMAND}" -E rm -rf "${report_dir}"
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${report_dir}"
        COMMAND "${GCOVR_EXECUTABLE}" ${gcovr_args} "${PROJECT_BINARY_DIR}"
        WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
        VERBATIM
    )
endfunction()

pominal_add_coverage_target()
