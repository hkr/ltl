cmake_minimum_required (VERSION 2.8.11)

project(ltl_test)
add_executable(ltl_test src/main.cpp)
target_link_libraries (ltl_test ltl)
target_include_directories(ltl_test PRIVATE src)

if (MSVC)
	# this is only needed when using ASM (boost.context), but for not let's set it also when using Fibers
	set_target_properties(ltl_test PROPERTIES LINK_FLAGS "/SAFESEH:NO") 
endif()