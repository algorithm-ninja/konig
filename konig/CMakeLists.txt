set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

add_executable(
    test_all

    tests/test_all.cpp
)
include_directories(include)
add_custom_command(TARGET test_all POST_BUILD COMMAND test_all)