
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
