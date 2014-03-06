cmake_minimum_required(VERSION 2.8.11)

project(boost_context CXX ASM)

file(GLOB_RECURSE boost_dependencies_public_headers  ${CMAKE_CURRENT_SOURCE_DIR} dependencies/*.hpp)
file(GLOB_RECURSE boost_context_public_headers       ${CMAKE_CURRENT_SOURCE_DIR} context/include/*.hpp)

if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(boost_context_arch_default x86_64)
else()
    set(boost_context_arch_default i386)
endif()

set(boost_context_arch ${boost_context_arch_default} CACHE STRING "Select target architecture")
set_property(CACHE boost_context_arch PROPERTY STRINGS x86_64 i386_x86 arm mips32 ppc32 ppc64 sparc sparc64)
message(STATUS "Boost.Context target architecture: ${boost_context_arch}")

if (${boost_context_arch} STREQUAL arm)
    set(boost_context_asm armasm)
    set(boost_context_ext asm)
elseif (WIN32)
    enable_language(ASM_MASM)
    set(boost_context_asm masm)
    set(boost_context_ext asm)
else()
    set(boost_context_asm gas)
    set(boost_context_ext S)
endif()
    
if (WIN32)
    set(boost_context_bin pe)
elseif (APPLE)
    set(boost_context_bin macho)
else()
    set(boost_context_bin elf)
endif()

if (WIN32)
    set(boost_context_cc ms)
elseif (${boost_context_arch} STREQUAL arm)
    set(boost_context_cc aapcs)
elseif (${boost_context_arch} STREQUAL mips32)
    set(boost_context_cc o32)
else()
    set(boost_context_cc sysv)
endif()

set(boost_context_filename_suffix
    ${boost_context_arch}_${boost_context_cc}_${boost_context_bin}_${boost_context_asm}.${boost_context_ext}
)

message(STATUS "Boost.Context looking for ASM files matching *_${boost_context_filename_suffix}")

set(boost_context_asm_sources 
    context/src/asm/jump_${boost_context_filename_suffix}
    context/src/asm/make_${boost_context_filename_suffix})

if(NOT boost_context_asm_sources)
    message(FATAL_ERROR "Boost.Context not supported for this platform, this can probably be be fixed by adapting the CMakeLists.txt of boost_context.")
endif()

add_library(boost_context 
    context/src/dummy.cpp 
    ${boost_context_asm_sources} 
    ${boost_context_public_headers} 
    ${boost_dependencies_public_headers})

target_include_directories(boost_context PUBLIC context/include dependencies PRIVATE src)
