
InitLib()

target_link_libraries(Renderer
PUBLIC
	libglew_static
	Util
PRIVATE
	imgui
	glm
	entt
)

add_precompiled_header(Renderer
	${CMAKE_CURRENT_SOURCE_DIR}/pch.hpp
	SOURCE_CXX ${CMAKE_CURRENT_SOURCE_DIR}/pch.cpp)
