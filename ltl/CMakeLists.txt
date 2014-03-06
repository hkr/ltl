cmake_minimum_required(VERSION 2.8.11)

project(ltl)
file(GLOB_RECURSE ltl_sources         ${CMAKE_CURRENT_SOURCE_DIR} src/ltl/*.cpp)
file(GLOB_RECURSE ltl_private_headers ${CMAKE_CURRENT_SOURCE_DIR} src/ltl/*.hpp)
file(GLOB_RECURSE ltl_public_headers  ${CMAKE_CURRENT_SOURCE_DIR} include/ltl/*.hpp)

add_library(ltl ${ltl_sources} ${ltl_public_headers} ${ltl_private_headers})

find_package(Threads)
target_link_libraries(ltl ltlcontext ${CMAKE_THREAD_LIBS_INIT})

target_include_directories(ltl PUBLIC include PRIVATE src)
