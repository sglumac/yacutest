add_executable(tests4tests tests.c others.c assertions.c failures.c)
target_include_directories(tests4tests PRIVATE .)
target_link_libraries(tests4tests yacu)
