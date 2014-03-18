cmake_minimum_required (VERSION 2.8.11)

project(lio_test)
add_executable(lio_test src/main.cpp)
target_link_libraries (lio_test lio)
target_include_directories(lio_test PRIVATE src)

if (MSVC)
	# this is only needed when using ASM (boost.context), but for not let's set it also when using Fibers
	set_target_properties(lio_test PROPERTIES LINK_FLAGS "/SAFESEH:NO") 
endif()
