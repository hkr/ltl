cmake_minimum_required(VERSION 2.8.11)

project(boost_context CXX ASM)
file(GLOB_RECURSE boost_dependencies_public_headers  ${CMAKE_CURRENT_SOURCE_DIR} dependencies/*.hpp)
file(GLOB_RECURSE boost_context_public_headers       ${CMAKE_CURRENT_SOURCE_DIR} context/include/*.hpp)

set(boost_context_asm_sources)

# OS X AMD64
set(boost_context_asm_sources 
	context/src/asm/jump_x86_64_sysv_macho_gas.S 
	context/src/asm/make_x86_64_sysv_macho_gas.S)

add_library(boost_context 
	context/src/dummy.cpp 
	${boost_context_asm_sources} 
	${boost_context_public_headers} 
	${boost_dependencies_public_headers})

target_include_directories(boost_context PUBLIC context/include dependencies PRIVATE src)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
