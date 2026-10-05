
InitLib()

target_link_libraries(Loader
PUBLIC
	HAL
	tinygltf
	par
PRIVATE
	Util
	Jobs
	ECS
)

add_precompiled_header(Loader
	${CMAKE_CURRENT_SOURCE_DIR}/pch.hpp
	SOURCE_CXX ${CMAKE_CURRENT_SOURCE_DIR}/pch.cpp)
