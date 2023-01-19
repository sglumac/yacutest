add_library(calculator STATIC)


target_sources(calculator PRIVATE calculator.c)
target_include_directories(calculator PUBLIC .)
