
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/DemoInfra")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/ECS")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/Engine")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/Loader")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/Messaging")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/NeoMain")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/Renderer")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/ResourceManager")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/Util")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/Hardware")

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/ext")

## Shaders
add_custom_target(Neo_GLSL)
file(GLOB_RECURSE SHADERS ${CMAKE_CURRENT_SOURCE_DIR}/shaders/*)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/shaders" FILES ${SHADERS})
set_property(TARGET Neo_GLSL APPEND PROPERTY SOURCES ${SHADERS})
set_target_properties(Neo_GLSL PROPERTIES FOLDER "Neo")

## Res
add_custom_target(Neo_Resources)
file(GLOB_RECURSE RESOURCES ${CMAKE_CURRENT_SOURCE_DIR}/res/*)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/res" FILES ${RESOURCES})
set_property(TARGET Neo_Resources APPEND PROPERTY SOURCES ${RESOURCES})
set_target_properties(Neo_Resources PROPERTIES FOLDER "Neo")

add_library(Neo INTERFACE)
target_link_libraries(Neo
INTERFACE
	ECS
	Engine
	Loader
	Messaging
	Renderer
	ResourceManager
	Hardware
	DemoInfra
	Util
	glfw
	glm
	libglew_static
	par
)


