
InitLib()

target_link_libraries(Util
PUBLIC
	glfw
	microprofile
	glm
	entt
PRIVATE
	imgui
)

add_precompiled_header(Util 
	${CMAKE_CURRENT_SOURCE_DIR}/pch.hpp
	SOURCE_CXX ${CMAKE_CURRENT_SOURCE_DIR}/pch.cpp)
