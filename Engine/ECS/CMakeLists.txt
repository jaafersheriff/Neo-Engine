
InitLib()

target_link_libraries(ECS
PUBLIC
	Util
	Messaging
	microprofile
	Hardware
	entt
PRIVATE
	libglew_static
)
