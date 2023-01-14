cmake_minimum_required(VERSION 3.16)
project(yacutest)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

if (COVERAGE STREQUAL "True")
    include(CodeCoverage)
    append_coverage_compiler_flags()
    setup_target_for_coverage_gcovr_html(
        NAME yacucoverage
        EXECUTABLE tests --report tests.log --no-fork
        DEPENDENCIES tests
        BASE_DIRECTORY ${PROJECT_SOURCE_DIR}
    )
endif()

add_subdirectory(src)
add_subdirectory(tests4tests)
