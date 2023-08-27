cmake_minimum_required(VERSION 3.16)
project(yacutest)

if(MSVC)
  add_compile_options(/W4 /WX)
else()
  add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

if(YACU_CMAKE_COVERAGE STREQUAL "True")
  include(CodeCoverage)
  append_coverage_compiler_flags()
  setup_target_for_coverage_gcovr_html(
    NAME yacucoverage
    EXECUTABLE tests4tests --junit tests4tests.xml
    DEPENDENCIES tests4tests
    BASE_DIRECTORY ${PROJECT_SOURCE_DIR}
  )
endif()

add_subdirectory(src)

if(YACU_CMAKE_TESTS4TESTS STREQUAL "True")
  add_subdirectory(tests4tests)
endif()
