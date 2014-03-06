cmake_minimum_required(VERSION 2.8.11)

project(lio)
file(GLOB_RECURSE lio_sources         ${CMAKE_CURRENT_SOURCE_DIR} src/lio/*.cpp)
file(GLOB_RECURSE lio_private_headers ${CMAKE_CURRENT_SOURCE_DIR} src/lio/*.hpp)
file(GLOB_RECURSE lio_public_headers  ${CMAKE_CURRENT_SOURCE_DIR} include/lio/*.hpp)

add_library(lio ${lio_sources} ${lio_public_headers} ${lio_private_headers})

find_package(Libuv)
target_link_libraries(lio ltl)

target_include_directories(lio PUBLIC include PRIVATE src)
