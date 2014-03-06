cmake_minimum_required(VERSION 2.8.11)

project(ltl_all)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/CMake/CXX11/")
include(CXX11)
enable_cxx11()

add_subdirectory(ltlcontext)
add_subdirectory(ltl)
add_subdirectory(ltl_test)

