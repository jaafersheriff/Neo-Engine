

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

	## Capstone
	add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/capstone")
	target_compile_options(capstone PRIVATE "/WX-" "/W0")
	set_target_properties(capstone PROPERTIES FOLDER "Neo/ext")
	target_include_directories(capstone
	PUBLIC
 		"${CMAKE_CURRENT_SOURCE_DIR}/capstone/include/capstone"
	)

	## tracy server - can be disabled
	if (NOT NO_LOCAL_TRACY)
		option(TRACY_NO_BROADCAST "" ON)
		option(TRACY_NO_ROOT_WINDOW "" ON)
		file(GLOB TRACY_SERVER_CPP_FILES 
			tracy/server/*.cpp
			tracy/public/common/*.cpp
			tracy/profiler/src/Fonts.cpp
			tracy/nfd/nfd_win.cpp
			tracy/zstd/common/*.c
			tracy/zstd/compress/*.c
			tracy/zstd/decompress/*.c
			tracy/zstd/dictBuilder/*.c
		)
		file(GLOB TRACY_SERVER_HPP_FILES 
			tracy/server/*.hpp
			tracy/public/common/*.hpp
			tracy/profiler/src/Fonts.hpp
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
		)

	unset(MESSAGE_QUIET)
	endif()
endif()

## imgui
file(GLOB IMGUI_CPP_FILES 
	imgui/*.cpp
	implot/*.cpp
)
file(GLOB IMGUI_HPP_FILES 
	imgui/*.h
	implot/*.h
	imgui_entt_entity_editor/*.hpp
)
add_library(imgui STATIC
	${IMGUI_CPP_FILES}
 	imgui/backends/imgui_impl_glfw.cpp 
 	imgui/backends/imgui_impl_opengl3.cpp 
	${IMGUI_HPP_FILES}
 	imgui/backends/imgui_impl_glfw.h
 	imgui/backends/imgui_impl_opengl3.h
)
set_target_properties(imgui PROPERTIES FOLDER "Neo/ext")
target_include_directories(imgui 
PUBLIC
	"${CMAKE_CURRENT_SOURCE_DIR}/imgui/"
 	"${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/"
 	"${CMAKE_CURRENT_SOURCE_DIR}/implot"
 	"${CMAKE_CURRENT_SOURCE_DIR}/imgui_entt_entity_editor"
)
target_link_libraries(imgui
PUBLIC
PRIVATE
	glfw
	entt
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
