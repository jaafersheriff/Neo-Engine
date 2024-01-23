
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

if (NO_LOCAL_TRACY) 
	target_compile_definitions(Util
	PRIVATE
		NO_LOCAL_TRACY
	)
else()
	target_link_libraries(Util
	PRIVATE
		TracyServer
	)
endif()

	


add_precompiled_header(Util 
	${CMAKE_CURRENT_SOURCE_DIR}/pch.hpp
	SOURCE_CXX ${CMAKE_CURRENT_SOURCE_DIR}/pch.cpp)
