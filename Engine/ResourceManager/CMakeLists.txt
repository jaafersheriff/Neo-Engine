
InitLib()

target_link_libraries(ResourceManager
PUBLIC
	GLBackend
PRIVATE
	Jobs
	entt
	glm
	TracyClient
	libglew_static
	imgui
)
