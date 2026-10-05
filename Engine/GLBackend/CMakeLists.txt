
InitLib()

target_link_libraries(GLBackend
PUBLIC
	HAL
	Util
PRIVATE
	libglew_static
	TracyClient
)

add_precompiled_header(GLBackend
	${CMAKE_CURRENT_SOURCE_DIR}/pch.hpp
	SOURCE_CXX ${CMAKE_CURRENT_SOURCE_DIR}/pch.cpp)
