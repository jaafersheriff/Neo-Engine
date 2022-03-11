
InitLib()

target_link_libraries(ECS
PUBLIC
	Util
	microprofile
	Hardware
	entt
PRIVATE
	libglew_static
)
