
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/DemoInfra")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/ECS")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/Engine")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/Loader")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/Messaging")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/NeoMain")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/Renderer")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/Util")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/Hardware")

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/ext")

add_custom_target(GLSL)
file(GLOB_RECURSE SHADERS ${CMAKE_CURRENT_SOURCE_DIR}/shaders/*)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/shaders" FILES ${SHADERS})
set_property(TARGET GLSL APPEND PROPERTY SOURCES ${SHADERS})
set_target_properties(GLSL PROPERTIES FOLDER "Neo")

add_library(Neo INTERFACE)
target_link_libraries(Neo
INTERFACE
	ECS
	Engine
	Loader
	Messaging
	Renderer
	Hardware
	DemoInfra
	Util
	glfw
	glm
	libglew_static
	TracyClient
)


