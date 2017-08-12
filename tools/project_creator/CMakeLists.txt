cmake_minimum_required(VERSION 3.3)

project(EleganceHelloworld)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g -ggdb -O0 -pthread -Wall -Werror")

set(ELEGANCE_SRC_PATH "include/")
set(ELEGANCE_LIB_PATH "lib/linux64")

file(GLOB_RECURSE SRC_FILES ./src/*.cpp)

LINK_DIRECTORIES(${ELEGANCE_LIB_PATH})

add_executable(EleganceHelloworld ${SRC_FILES})

target_include_directories(EleganceHelloworld PUBLIC ${ELEGANCE_SRC_PATH} ./src )
target_link_libraries(EleganceHelloworld libElegance.so)



