
InitLib()

target_link_libraries(ECS
PUBLIC
	Util
	microprofile
	Hardware
	EnTT::EnTT
PRIVATE
	libglew_static
)
