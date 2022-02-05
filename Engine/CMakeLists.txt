
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/ECS")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/Engine")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/Loader")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/Messaging")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/Renderer")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/Util")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/Window")

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/ext")

add_library(Neo INTERFACE)
target_link_libraries(Neo
INTERFACE
	ECS
	Engine
	Loader
	Messaging
	Renderer
	Window
	Util
	glfw
	libglew_static
)

add_custom_target(shaders)
file(GLOB_RECURSE SHADERS ${CMAKE_CURRENT_SOURCE_DIR}/shaders/*)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/shaders" FILES ${SHADERS})
set_property(TARGET shaders APPEND PROPERTY SOURCES ${SHADERS})
set_target_properties(shaders PROPERTIES FOLDER "Neo/shaders")


