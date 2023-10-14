
InitLib()

target_link_libraries(Loader
PUBLIC
PRIVATE
	Util
	libglew_static
	TracyClient
)

add_precompiled_header(Loader
	${CMAKE_CURRENT_SOURCE_DIR}/pch.hpp
	SOURCE_CXX ${CMAKE_CURRENT_SOURCE_DIR}/pch.cpp)
