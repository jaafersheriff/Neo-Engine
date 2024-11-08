
message(STATUS "Generating Neo/ext")

## GLFW
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/glfw")
target_include_directories(glfw AFTER 
PRIVATE 
)
set_target_properties(glfw PROPERTIES FOLDER "Neo/ext/GLFW3")
set_target_properties(update_mappings PROPERTIES FOLDER "Neo/ext/GLFW3")

## GLEW
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/glew-cmake")
set_target_properties(libglew_shared PROPERTIES FOLDER "Neo/ext/glew")
set_target_properties(libglew_static PROPERTIES FOLDER "Neo/ext/glew")

## GLM
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/glm")

## tracy
if (MSVC) 

	# Quiet the spew
	function(message)
	    if (NOT MESSAGE_QUIET)
	        _message(${ARGN})
	    endif()
	endfunction()
	set(MESSAGE_QUIET ON)

	option(TRACY_ENABLE "" ON)
	option(TRACY_ON_DEMAND "" ON)
	add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/tracy")
	target_compile_options(TracyClient PRIVATE "/WX-" "/W0")
	set_target_properties(TracyClient PROPERTIES FOLDER "Neo/ext")
	unset(MESSAGE_QUIET)
endif()

## imgui
file(GLOB IMGUI_CPP_FILES 
	imgui/*.cpp
	ImGuizmo/*.cpp
	implot/*.cpp
)
file(GLOB IMGUI_HPP_FILES 
	imgui_config.hpp
	imgui_incl.hpp
	imgui/*.h
	imgui_entt_entity_editor/*.hpp
	ImGuizmo/*.h
	implot/*.h
)
add_library(imgui STATIC
	${IMGUI_CPP_FILES}
 	imgui/backends/imgui_impl_glfw.cpp 
	${IMGUI_HPP_FILES}
 	imgui/backends/imgui_impl_glfw.h
)
target_compile_options(imgui PRIVATE "/WX-" "/W0")
set_target_properties(imgui PROPERTIES FOLDER "Neo/ext")
target_include_directories(imgui 
PUBLIC
	"${CMAKE_CURRENT_SOURCE_DIR}/imgui/"
 	"${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/"
 	"${CMAKE_CURRENT_SOURCE_DIR}/imgui_entt_entity_editor"
 	"${CMAKE_CURRENT_SOURCE_DIR}/ImGuizmo"
 	"${CMAKE_CURRENT_SOURCE_DIR}/implot"
PRIVATE
 	"${CMAKE_CURRENT_SOURCE_DIR}"
)
target_link_libraries(imgui
PUBLIC
	Util
PRIVATE
	glfw
	entt
)
target_compile_definitions(imgui
PRIVATE
	IMGUI_USER_CONFIG="imgui_config.hpp"
)

## entt
file(GLOB_RECURSE ENTT_HPP_FILES 
	entt/src/*.h
	entt/src/*.hpp
)
file(GLOB_RECURSE ENTT_CPP_FILES 
	entt.cpp
)
add_library(entt STATIC ${ENTT_CPP_FILES} ${ENTT_HPP_FILES} entt_incl.hpp)
source_group(
	TREE  ${CMAKE_CURRENT_SOURCE_DIR}/entt/src 
	FILES ${ENTT_HPP_FILES}
)
set_target_properties(entt PROPERTIES FOLDER "Neo/ext")
target_include_directories(entt
PUBLIC
	"${CMAKE_CURRENT_SOURCE_DIR}/entt/src"
PRIVATE
)
target_link_libraries(entt
PUBLIC
	Util
)


## par
file(GLOB_RECURSE PAR_HPP_FILES 
	par/par_shapes.h
)
file(GLOB_RECURSE PAR_CPP_FILES 
	par.cpp
)
add_library(par STATIC ${PAR_CPP_FILES} ${PAR_HPP_FILES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/par FILES 
		  ${PAR_HPP_FILES})
set_target_properties(par PROPERTIES FOLDER "Neo/ext")
target_include_directories(par
PUBLIC
	"${CMAKE_CURRENT_SOURCE_DIR}/par"
PRIVATE
)
target_compile_options(par PRIVATE "/WX-" "/W0")

## tiny gltf
set(TINYGLTF_BUILD_LOADER_EXAMPLE OFF CACHE BOOL "" FORCE)
set(TINYGLTF_INSTALL OFF CACHE BOOL "" FORCE)
set(TINYGLTF_HEADER_ONLY OFF CACHE BOOL "" FORCE)
file(GLOB_RECURSE TINYGLTF_HPP_FILES 
	tinygltf/json.hpp
	tinygltf/stb_image.h
	tinygltf/stb_image_write.h
	tinygltf/tiny_gltf.h
)
file(GLOB_RECURSE TINYGLTF_CPP_FILES 
	tiny_gltf.cc
)
add_library(tinygltf STATIC ${TINYGLTF_CPP_FILES} ${TINYGLTF_HPP_FILES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/tinygltf FILES 
		  ${TINYGLTF_HPP_FILES})
set_target_properties(tinygltf PROPERTIES FOLDER "Neo/ext")
target_include_directories(tinygltf
PUBLIC
	"${CMAKE_CURRENT_SOURCE_DIR}/tinygltf/"
PRIVATE
)
set_target_properties(tinygltf PROPERTIES FOLDER "Neo/ext/")
target_compile_options(tinygltf PRIVATE "/WX-" "/W0")
