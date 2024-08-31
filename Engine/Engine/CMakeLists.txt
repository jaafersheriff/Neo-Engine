
InitLib()

target_link_libraries(Engine
PUBLIC
PRIVATE
	Util
	Messaging
	Hardware
	glfw
	libglew_static
	imgui
	entt
	TracyClient
	implots
)

if (NOT NO_LOCAL_TRACY) 
	target_link_libraries(Engine
	PRIVATE
		TracyServer
	)
endif()


