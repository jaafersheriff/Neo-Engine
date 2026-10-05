
message(STATUS "Generating Neo/HAL")

# Header-only: the API-neutral types shared by the renderer backend and its users
file(GLOB_RECURSE HPP_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/*.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/*.h)

add_library(HAL INTERFACE ${HPP_FILES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${HPP_FILES})

target_include_directories(HAL
INTERFACE
	${CMAKE_SOURCE_DIR}/Engine
)

set_target_properties(HAL PROPERTIES FOLDER "Neo")
