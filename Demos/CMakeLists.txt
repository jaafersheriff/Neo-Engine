cmake_minimum_required(VERSION 3.18)

message(STATUS "Generating Demos")

project(Demos)
add_library(Demos)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB_RECURSE CPP_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
file(GLOB_RECURSE HPP_FILES 
		  ${CMAKE_CURRENT_SOURCE_DIR}/*.h
		  ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES 
	${CPP_FILES} 
	${HPP_FILES})

target_sources(Demos
PRIVATE
	${CPP_FILES}
	${HPP_FILES}
)

target_include_directories(Demos
PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}
PRIVATE
)


target_link_libraries(Demos
PUBLIC
PRIVATE
	Neo
)

