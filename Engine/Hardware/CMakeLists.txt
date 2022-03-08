
InitLib()

target_link_libraries(Hardware
PUBLIC
	libglew_static
PRIVATE
	Util
	imgui
	glfw
	EnTT::EnTT
)
