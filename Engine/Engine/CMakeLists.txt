
InitLib()

target_link_libraries(Engine
PUBLIC
	Util
	Window
PRIVATE
	libglew_static
)
