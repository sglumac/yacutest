add_library(yacu STATIC yacu.c)

target_include_directories(yacu PUBLIC .)

if(BUILD_FORKED_TESTS STREQUAL "True")
  message(BUILD_FORKED_TESTS="${BUILD_FORKED_TESTS}")
  target_compile_definitions(yacu PRIVATE FORK_TESTS)
endif()
