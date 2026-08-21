
InitLib()

target_link_libraries(Engine
PUBLIC
PRIVATE
	Util
	Jobs
	Messaging
	Hardware
	glfw
	libglew_static
	imgui
	entt
	TracyClient
)


