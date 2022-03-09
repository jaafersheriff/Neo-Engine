
InitLib()

target_link_libraries(Renderer
PUBLIC
	libglew_static
	Util
PRIVATE
	imgui
	glm
	EnTT::EnTT
)
