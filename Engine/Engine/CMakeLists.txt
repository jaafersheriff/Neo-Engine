
InitLib()

target_link_libraries(Engine
PUBLIC
PRIVATE
	Util
	Messaging
	Hardware
	glfw
	libglew_static
	imgui
	entt
	TracyClient
)

if (NO_LOCAL_TRACY) 
	target_compile_definitions(Engine
	PRIVATE
		NO_LOCAL_TRACY
	)
else()
	target_link_libraries(Engine
	PRIVATE
		TracyServerFonts
	)
endif()

