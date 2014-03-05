cmake_minimum_required(VERSION 2.8.11)

set(ltl_context_backend_lib_default ucontext)
set(ltl_ContextBackendLib ${ltl_context_backend_lib_default} CACHE STRING "Select underlying contexts library to use")
set_property(CACHE ltl_ContextBackendLib PROPERTY STRINGS ucontext boostcontext fibers)
message(STATUS "ltlcontext used backend library: ${ltl_ContextBackendLib}")

project(ltlcontext)
file(GLOB_RECURSE ltlcontext_sources ${CMAKE_CURRENT_SOURCE_DIR} src/ltlcontext/${ltl_ContextBackendLib}/*.cpp)
file(GLOB_RECURSE ltlcontext_private_headers ${CMAKE_CURRENT_SOURCE_DIR} src/ltlcontext/*.hpp)
file(GLOB_RECURSE ltlcontext_public_headers  ${CMAKE_CURRENT_SOURCE_DIR} include/ltlcontext/*.hpp)

add_library(ltlcontext ${ltlcontext_sources} ${ltlcontext_public_headers} ${ltlcontext_private_headers})
target_include_directories(ltlcontext PUBLIC include PRIVATE src)

if(${ltl_ContextBackendLib} STREQUAL boostcontext)
    add_subdirectory(boost_context)
    target_link_libraries(ltlcontext boost_context)
endif()

if(${ltl_ContextBackendLib} STREQUAL ucontext)
    target_compile_definitions(ltlcontext PRIVATE "-D_XOPEN_SOURCE")
endif()

if (CMAKE_COMPILER_IS_GNUCXX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()
