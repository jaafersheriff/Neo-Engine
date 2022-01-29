
InitLib()

target_link_libraries(Engine
PUBLIC
	Util
PRIVATE
	libglew_static
	glfw
	glm
)
