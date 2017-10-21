cmake_minimum_required(VERSION 3.3)
project(elegance_demo)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g -ggdb -O0 -pthread -Wall -Werror")

set(ELEGANCE_SRC_PATH "../")
set(ELEGANCE_LIB_PATH "../lib/linux64")

file(GLOB_RECURSE ELEGANCE_DEMO_FILES src/*.cpp)

LINK_DIRECTORIES(${ELEGANCE_LIB_PATH})

add_executable(elegance_demo ${ELEGANCE_DEMO_FILES})

target_include_directories(elegance_demo PUBLIC ${ELEGANCE_SRC_PATH} ../common/ src/center )
target_link_libraries(elegance_demo libElegance.so)



