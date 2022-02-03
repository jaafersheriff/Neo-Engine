
InitLib()

target_link_libraries(Renderer
PUBLIC
	libglew_static
PRIVATE
	imgui
	Util
	glm
)
