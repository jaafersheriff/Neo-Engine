
InitLib()

target_link_libraries(Engine
PUBLIC
PRIVATE
	Util
	Messaging
	Hardware
	libglew_static
	imgui
	entt
	TracyClient
)
