
InitLib()

target_link_libraries(Hardware
PUBLIC
PRIVATE
	Util
	Messaging
	imgui
	glfw
	entt
	TracyClient
)

add_precompiled_header(Hardware 
	${CMAKE_CURRENT_SOURCE_DIR}/pch.hpp
	SOURCE_CXX ${CMAKE_CURRENT_SOURCE_DIR}/pch.cpp)
