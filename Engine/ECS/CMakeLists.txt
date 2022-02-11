
InitLib()

target_link_libraries(ECS
PUBLIC
	Util
	microprofile
	Hardware
PRIVATE
	libglew_static
)
