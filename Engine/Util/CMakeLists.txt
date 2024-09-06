
InitLib()

target_link_libraries(Util
PUBLIC
	glm
	entt
	imgui
	TracyClient
	libglew_static 
PRIVATE
	glfw
)

add_precompiled_header(Util 
	${CMAKE_CURRENT_SOURCE_DIR}/pch.hpp
	SOURCE_CXX ${CMAKE_CURRENT_SOURCE_DIR}/pch.cpp)
