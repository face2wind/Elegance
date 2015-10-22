cmake_minimum_required(VERSION 2.8)
project(Elegance)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g -ggdb -O0 -lboost_system -lprotobuf -lpthread -Wall -Werror")

file(GLOB_RECURSE SOURCE_FILES ../src/*.cpp)

find_package(Boost COMPONENTS system filesystem thread log program_options REQUIRED)
if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
endif()

find_package(Protobuf REQUIRED)
if(PROTOBUF_FOUND)
  include_directories(${PROTOBUF_INCLUDE_DIRS})
endif()

include_directories(
  ../src
  )

#add_executable(fileTransfer ${SOURCE_FILES} ${MAIN_CPP})
ADD_LIBRARY(Elegance_share SHARED ${SOURCE_FILES})
ADD_LIBRARY(Elegance_static STATIC ${SOURCE_FILES})
SET_TARGET_PROPERTIES(Elegance_share PROPERTIES OUTPUT_NAME "Elegance" VERSION 1.2 SOVERSION 1)
SET_TARGET_PROPERTIES(Elegance_static PROPERTIES OUTPUT_NAME "Elegance" VERSION 1.2 SOVERSION 1)

target_link_libraries(Elegance_share ${Boost_LIBRARIES} ${PROTOBUF_LIBRARIES})
target_link_libraries(Elegance_static ${Boost_LIBRARIES} ${PROTOBUF_LIBRARIES})

