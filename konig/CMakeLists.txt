set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES .)
include_directories(cpp-btree)
add_executable(test_all tests/test_all.cpp ${SOURCE_FILES})

