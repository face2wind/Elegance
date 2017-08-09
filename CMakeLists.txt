cmake_minimum_required(VERSION 2.8)
project(Elegance)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g -ggdb -O0 -pthread -Wall -Werror")

file(GLOB_RECURSE SOURCE_FILES ./elegance/*.cpp)

include_directories(
  .
  )

ADD_LIBRARY(Elegance_share SHARED ${SOURCE_FILES})
ADD_LIBRARY(Elegance_static STATIC ${SOURCE_FILES})

SET_TARGET_PROPERTIES(Elegance_share PROPERTIES OUTPUT_NAME "Elegance" VERSION 1.2 SOVERSION 1)
SET_TARGET_PROPERTIES(Elegance_static PROPERTIES OUTPUT_NAME "Elegance" VERSION 1.2 SOVERSION 1)

set_target_properties(Elegance_share PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/../lib/linux/)
set_target_properties(Elegance_static PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/../lib/linux/)

install(TARGETS Elegance_share Elegance_static
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib/static)
