set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES include/konig.hpp
                 src/konig.cpp
   )

add_executable(konig ${SOURCE_FILES})
add_executable(test_all tests/test_all.cpp ${SOURCE_FILES})

find_package(PythonLibs REQUIRED)
include_directories(${PYTHON_INCLUDE_DIRS})
target_link_libraries(konig ${PYTHON_LIBRARIES})
target_link_libraries(test_all ${PYTHON_LIBRARIES})

