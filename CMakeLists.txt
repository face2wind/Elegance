cmake_minimum_required(VERSION 2.8)
project(elegance)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g -ggdb -O0 -lboost_system -lprotobuf -lpthread -Wall -Werror")

file(GLOB_RECURSE SOURCE_FILES ./src/*.cpp)

include_directories(
  ./src
  )

SET_TARGET_PROPERTIES(Elegance_share PROPERTIES OUTPUT_NAME "elegance" VERSION 1.2 SOVERSION 1)
SET_TARGET_PROPERTIES(Elegance_static PROPERTIES OUTPUT_NAME "elegance" VERSION 1.2 SOVERSION 1)

target_link_libraries(Elegance_share)
target_link_libraries(Elegance_static)

