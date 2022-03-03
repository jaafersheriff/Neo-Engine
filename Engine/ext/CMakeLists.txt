

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

## microprofile
set(MICROPROFILE_USE_CONFIG_FILE ON CACHE BOOL "" FORCE)
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/microprofile")
set_target_properties(microprofile PROPERTIES FOLDER "Neo/ext")
target_link_libraries(microprofile
PRIVATE
	libglew_static
)
target_include_directories(microprofile
PUBLIC
	"${CMAKE_CURRENT_SOURCE_DIR}"
)
target_sources(microprofile PRIVATE "microprofile.config.h")

## imgui
file(GLOB IMGUI_CPP_FILES imgui/*.cpp)
file(GLOB IMGUI_HPP_FILES imgui/*.h)
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
PRIVATE
)
target_link_libraries(imgui
PUBLIC
PRIVATE
	glfw
)

## implot
file(GLOB IMPLOT_CPP_FILES imgui/*.cpp)
file(GLOB IMPLOT_HPP_FILES imgui/*.h)
add_library(implot STATIC
	${IMGUI_CPP_FILES}
	${IMGUI_HPP_FILES}
)
set_target_properties(implot PROPERTIES FOLDER "Neo/ext")
target_include_directories(implot 
PUBLIC
	"${CMAKE_CURRENT_SOURCE_DIR}/implot/"
PRIVATE
)


