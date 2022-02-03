
InitLib()

target_link_libraries(Window
PUBLIC
	libglew_static
PRIVATE
	Util
	imgui
	glfw
)
