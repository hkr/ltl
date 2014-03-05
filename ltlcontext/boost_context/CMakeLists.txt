cmake_minimum_required(VERSION 2.8.11)

project(boost_context CXX ASM)

file(GLOB_RECURSE boost_dependencies_public_headers  ${CMAKE_CURRENT_SOURCE_DIR} dependencies/*.hpp)
file(GLOB_RECURSE boost_context_public_headers       ${CMAKE_CURRENT_SOURCE_DIR} context/include/*.hpp)

if(APPLE)
	if (CMAKE_SIZEOF_VOID_P EQUAL 8)
		set(boost_context_asm_sources 
			context/src/asm/jump_x86_64_sysv_macho_gas.S 
			context/src/asm/make_x86_64_sysv_macho_gas.S)
	endif()
elseif(WIN32)
	enable_language(ASM_MASM)
	if (CMAKE_SIZEOF_VOID_P EQUAL 4)
		set(boost_context_asm_sources 
			context/src/asm/jump_i386_ms_pe_masm.asm 
			context/src/asm/make_i386_ms_pe_masm.asm)
	elseif(CMAKE_SIZEOF_VOID_P EQUAL 8)
		set(boost_context_asm_sources 
			context/src/asm/jump_x86_64_ms_pe_masm.asm 
			context/src/asm/make_x86_64_ms_pe_masm.asm)
	endif()
	
elseif(UNIX)

endif()

if(NOT boost_context_asm_sources)
	message(FATAL_ERROR "Boost.Context not supported for this platform, this can probably be be fixed by adapting the CMakeLists.txt of boost_context.")
endif()

add_library(boost_context 
	context/src/dummy.cpp 
	${boost_context_asm_sources} 
	${boost_context_public_headers} 
	${boost_dependencies_public_headers})

target_include_directories(boost_context PUBLIC context/include dependencies PRIVATE src)
