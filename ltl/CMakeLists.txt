cmake_minimum_required(VERSION 2.8.11)

project(ltl)
file(GLOB_RECURSE ltl_sources src *.cpp)
file(GLOB_RECURSE ltl_private_headers src *.hpp)
file(GLOB_RECURSE ltl_public_headers include *.hpp)

add_library(ltl ${ltl_sources} ${ltl_public_headers} ${ltl_private_headers})

target_include_directories(ltl PUBLIC include PRIVATE src)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
