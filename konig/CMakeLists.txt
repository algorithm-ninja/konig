set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES .)
add_executable(test_all tests/test_all.cpp ${SOURCE_FILES})

