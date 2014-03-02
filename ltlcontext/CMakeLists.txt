cmake_minimum_required(VERSION 2.8.11)

add_subdirectory(boost_context)

project(ltlcontext)
file(GLOB_RECURSE ltlcontext_sources ${CMAKE_CURRENT_SOURCE_DIR} src/ltlcontext/*.cpp)
file(GLOB_RECURSE ltlcontext_private_headers ${CMAKE_CURRENT_SOURCE_DIR} src/ltlcontext/*.hpp)
file(GLOB_RECURSE ltlcontext_public_headers  ${CMAKE_CURRENT_SOURCE_DIR} include/ltlcontext/*.hpp)

add_library(ltlcontext ${ltlcontext_sources} ${ltlcontext_public_headers} ${ltlcontext_private_headers})
target_link_libraries(ltlcontext boost_context)

target_include_directories(ltlcontext PUBLIC include PRIVATE src)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
