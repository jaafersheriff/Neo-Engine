
InitLib()

target_link_libraries(ECS
PUBLIC
	Util
	microprofile
	Window
PRIVATE
	libglew_static
)
