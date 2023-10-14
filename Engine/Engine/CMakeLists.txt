
InitLib()

target_link_libraries(Engine
PUBLIC
	Util
	Messaging
	Hardware
PRIVATE
	libglew_static
	imgui
	entt
	TracyClient
	TracyServer
)
