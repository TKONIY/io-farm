project(co_rpc)
cmake_minimum_required(VERSION 3.20)

find_package(fmt)
include_directories(${FMT_INCLUDE_DIR})

set(DYNAMIC_LINK_LIB fmt::fmt)

add_executable(server ${CMAKE_CURRENT_SOURCE_DIR}/src/server.cpp)
target_include_directories(server PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(server fmt::fmt)