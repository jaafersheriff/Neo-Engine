
InitLib()

target_link_libraries(Hardware
PUBLIC
	libglew_static
PRIVATE
	Util
	Messaging
	imgui
	glfw
	entt
)
