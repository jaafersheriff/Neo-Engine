
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

	## tracy server - can be disabled
	if (NOT NO_LOCAL_TRACY)
		option(TRACY_NO_BROADCAST "" ON)
		option(TRACY_NO_ROOT_WINDOW "" ON)

		## Capstone
		add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/capstone")
		target_compile_options(capstone PRIVATE "/WX-" "/W0")
		set_target_properties(capstone PROPERTIES FOLDER "Neo/ext")
		target_include_directories(capstone
		PUBLIC
 			"${CMAKE_CURRENT_SOURCE_DIR}/capstone/include/capstone"
		)

		file(GLOB TRACY_SERVER_FONTS_CPP_FILES
			tracy/profiler/src/Fonts.cpp
		)
		file(GLOB TRACY_SERVER_FONTS_HPP_FILES
			tracy/profiler/src/Fonts.Hpp
		)
		add_library(TracyServerFonts STATIC
			${TRACY_SERVER_FONTS_CPP_FILES}
			${TRACY_SERVER_FONTS_HPP_FILES}
		)
		target_include_directories(TracyServerFonts
		PUBLIC
		 	"${CMAKE_CURRENT_SOURCE_DIR}/tracy/profiler/src"
		)
		target_compile_options(TracyServerFonts PUBLIC "/WX-" "/W0")
		target_compile_definitions(TracyServerFonts PRIVATE
			TRACY_ENABLE
			TRACY_NO_BROADCAST
			TRACY_NO_ROOT_WINDOW
		)
		set_target_properties(TracyServerFonts PROPERTIES FOLDER "Neo/ext")
		target_link_libraries(TracyServerFonts
		PRIVATE
			imgui
			capstone
		)

		file(GLOB TRACY_SERVER_CPP_FILES 
			tracy/server/*.cpp
			tracy/public/common/*.cpp
			tracy/profiler/src/imgui/*.cpp
			tracy/nfd/nfd_win.cpp
			tracy/zstd/common/*.c
			tracy/zstd/compress/*.c
			tracy/zstd/decompress/*.c
			tracy/zstd/dictBuilder/*.c
		)
		file(GLOB TRACY_SERVER_HPP_FILES 
			tracy/server/*.hpp
			tracy/public/common/*.hpp
			tracy/profiler/src/imgui/*.h
			tracy/nfd/nfd.h
			tracy/zstd/*.h
			tracy/zstd/common/*.h
			tracy/zstd/compress/*.h
			tracy/zstd/decompress/*.h
			tracy/zstd/dictBuilder/*.h
		)
		add_library(TracyServer STATIC
			${TRACY_SERVER_CPP_FILES}
			${TRACY_SERVER_HPP_FILES}
		)
		target_include_directories(TracyServer
		PUBLIC
		 	"${CMAKE_CURRENT_SOURCE_DIR}/tracy/server"
		)
		target_compile_options(TracyServer PUBLIC "/WX-" "/W0")
		target_compile_definitions(TracyServer PRIVATE
			TRACY_ENABLE
			TRACY_NO_BROADCAST
			TRACY_NO_ROOT_WINDOW
		)
		set_target_properties(TracyServer PROPERTIES FOLDER "Neo/ext")
		target_link_libraries(TracyServer
		PRIVATE
			imgui
			capstone
			glfw
		)


	endif()
	unset(MESSAGE_QUIET)
endif()

## imgui
file(GLOB IMGUI_CPP_FILES 
	imgui/*.cpp
	ImGuizmo/*.cpp
)
file(GLOB IMGUI_HPP_FILES 
	imgui/*.h
	imgui_entt_entity_editor/*.hpp
	ImGuizmo/*.h
	imgui_config.hpp
	imgui_incl.hpp
)
add_library(imgui STATIC
	${IMGUI_CPP_FILES}
 	imgui/backends/imgui_impl_glfw.cpp 
 	imgui/backends/imgui_impl_opengl3.cpp 
	${IMGUI_HPP_FILES}
 	imgui/backends/imgui_impl_glfw.h
 	imgui/backends/imgui_impl_opengl3.h
)
target_compile_options(imgui PRIVATE "/WX-" "/W0")
set_target_properties(imgui PROPERTIES FOLDER "Neo/ext")
target_include_directories(imgui 
PUBLIC
	"${CMAKE_CURRENT_SOURCE_DIR}/imgui/"
 	"${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/"
 	"${CMAKE_CURRENT_SOURCE_DIR}/imgui_entt_entity_editor"
 	"${CMAKE_CURRENT_SOURCE_DIR}/ImGuizmo"
)
target_link_libraries(imgui
PUBLIC
PRIVATE
	glfw
	entt
)

## implots
file(GLOB IMPLOTS_CPP_FILES 
	implot/*.cpp
)
file(GLOB IMPLOTS_HPP_FILES 
	implot/*.h
)
add_library(implots STATIC
	${IMPLOTS_CPP_FILES}
	${IMPLOTS_HPP_FILES}
)
set_target_properties(implots PROPERTIES FOLDER "Neo/ext")
target_include_directories(implots
PUBLIC
	"${CMAKE_CURRENT_SOURCE_DIR}/imgui/"
 	"${CMAKE_CURRENT_SOURCE_DIR}/implot"
)
target_link_libraries(implots
PUBLIC
PRIVATE
	glfw
)

## entt
file(GLOB_RECURSE ENTT_HPP_FILES 
	entt/src/*.h
	entt/src/*.hpp
)
file(GLOB_RECURSE ENTT_CPP_FILES 
	entt.cpp
)
add_library(entt STATIC ${ENTT_CPP_FILES} ${ENTT_HPP_FILES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/entt/src FILES 
	${ENTT_HPP_FILES})
set_target_properties(entt PROPERTIES FOLDER "Neo/ext")
target_include_directories(entt
PUBLIC
	"${CMAKE_CURRENT_SOURCE_DIR}/entt/src"
PRIVATE
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
