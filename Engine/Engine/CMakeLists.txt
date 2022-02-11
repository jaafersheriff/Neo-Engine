
InitLib()

target_link_libraries(Engine
PUBLIC
	Util
	Hardware
PRIVATE
	libglew_static
)
