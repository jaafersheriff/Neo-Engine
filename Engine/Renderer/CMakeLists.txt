
InitLib()

target_link_libraries(Renderer
PUBLIC
	GLBackend
	HAL
PRIVATE
	Util
	glfw
	imgui
	glm
	entt
	TracyClient
)

add_precompiled_header(Renderer
	${CMAKE_CURRENT_SOURCE_DIR}/pch.hpp
	SOURCE_CXX ${CMAKE_CURRENT_SOURCE_DIR}/pch.cpp)
