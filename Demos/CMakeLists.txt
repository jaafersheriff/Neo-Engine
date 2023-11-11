cmake_minimum_required(VERSION 3.18)

message(STATUS "Generating Demos")

project(Demos)
add_library(Demos)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB_RECURSE CPP_FILES 
# 	${CMAKE_CURRENT_SOURCE_DIR}/*/*.cpp
${CMAKE_CURRENT_SOURCE_DIR}/Base/*.cpp
${CMAKE_CURRENT_SOURCE_DIR}/Sponza/*.cpp
${CMAKE_CURRENT_SOURCE_DIR}/Compute/*.cpp
${CMAKE_CURRENT_SOURCE_DIR}/Cornell/*.cpp
${CMAKE_CURRENT_SOURCE_DIR}/NormalVisualizer/*.cpp
)
file(GLOB_RECURSE HPP_FILES 
	${CMAKE_CURRENT_SOURCE_DIR}/DemoRegistration.hpp
#	${CMAKE_CURRENT_SOURCE_DIR}/*/*.hpp
${CMAKE_CURRENT_SOURCE_DIR}/Base/*.hpp
${CMAKE_CURRENT_SOURCE_DIR}/Sponza/*.hpp
${CMAKE_CURRENT_SOURCE_DIR}/Compute/*.hpp
${CMAKE_CURRENT_SOURCE_DIR}/Cornell/*.hpp
${CMAKE_CURRENT_SOURCE_DIR}/NormalVisualizer/*.hpp
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

set_target_properties(Demos PROPERTIES FOLDER "Demos")

## Shaders
add_custom_target(Demo_GLSL)
file(GLOB_RECURSE DEMO_SHADERS ${CMAKE_CURRENT_SOURCE_DIR}/shaders/*)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/shaders" FILES ${DEMO_SHADERS})
set_property(TARGET Demo_GLSL APPEND PROPERTY SOURCES ${DEMO_SHADERS})
set_target_properties(Demo_GLSL PROPERTIES FOLDER "Demos")

