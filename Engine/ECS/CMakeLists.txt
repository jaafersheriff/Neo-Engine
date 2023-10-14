
InitLib()

target_link_libraries(ECS
PUBLIC
	Util
	Messaging
	Hardware
	entt
	imgui
PRIVATE
	libglew_static
	TracyClient
)

add_precompiled_header(ECS 
	${CMAKE_CURRENT_SOURCE_DIR}/pch.hpp
	SOURCE_CXX ${CMAKE_CURRENT_SOURCE_DIR}/pch.cpp)
