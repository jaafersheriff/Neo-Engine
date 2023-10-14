
InitLib()

target_link_libraries(Engine
PUBLIC
	Util
	Messaging
	Hardware
	TracyClient
	TracyServer
PRIVATE
	libglew_static
	imgui
	entt
)
