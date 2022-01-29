

## GLFW
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/glfw")
find_package(Vulkan REQUIRED)
target_include_directories(glfw AFTER 
PRIVATE 
)
set_target_properties(glfw PROPERTIES FOLDER "ext/GLFW3")
set_target_properties(update_mappings PROPERTIES FOLDER "ext/GLFW3")

## GLM
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/glm")

## imgui
file(GLOB CPP_FILES imgui/*.cpp)
file(GLOB HPP_FILES imgui/*.h)
add_library(imgui STATIC
	${CPP_FILES}
 	imgui/backends/imgui_impl_glfw.cpp 
 	imgui/backends/imgui_impl_opengl3.cpp 
	${HPP_FILES}
 	imgui/backends/imgui_impl_glfw.h
 	imgui/backends/imgui_impl_opengl3.h
)
set_target_properties(imgui PROPERTIES FOLDER "ext")

target_include_directories(imgui 
PUBLIC
	"${CMAKE_CURRENT_SOURCE_DIR}/imgui/"
 	"${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/"
PRIVATE
)

target_link_libraries(imgui
PUBLIC
PRIVATE
	glfw
)


