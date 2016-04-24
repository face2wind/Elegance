cmake_minimum_required(VERSION 2.8)
project(Elegance)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g -ggdb -O0 -pthread -Wall -Werror")

file(GLOB_RECURSE SOURCE_FILES ./src/*.cpp)

include_directories(
  ./src
  )

ADD_LIBRARY(Elegance_share SHARED ${SOURCE_FILES})
ADD_LIBRARY(Elegance_static STATIC ${SOURCE_FILES})

SET_TARGET_PROPERTIES(Elegance_share PROPERTIES OUTPUT_NAME "Elegance" VERSION 1.2 SOVERSION 1)
SET_TARGET_PROPERTIES(Elegance_static PROPERTIES OUTPUT_NAME "Elegance" VERSION 1.2 SOVERSION 1)


