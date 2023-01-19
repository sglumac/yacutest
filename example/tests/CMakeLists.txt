add_executable(tests tests.c)

include(FetchContent)

FetchContent_Declare(
  yacutest
  GIT_REPOSITORY https://github.com/sglumac/yacutest.git
  GIT_TAG main
)
FetchContent_MakeAvailable(yacutest)

target_link_libraries(tests calculator yacu)
