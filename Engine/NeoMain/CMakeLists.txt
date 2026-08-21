file(GLOB_RECURSE CPP_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
file(GLOB_RECURSE HPP_FILES 
		  ${CMAKE_CURRENT_SOURCE_DIR}/*.h
		  ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp
)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/${TargetID} FILES 
	${CPP_FILES} 
	${HPP_FILES})

add_executable(NeoMain ${CPP_FILES} ${HPP_FILES})
set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT NeoMain)
set_property(TARGET NeoMain PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/Demos/")

target_include_directories(NeoMain 
PUBLIC
PRIVATE 
	${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(NeoMain
PUBLIC
	Neo
PRIVATE
	Demos
)

set_target_properties(NeoMain PROPERTIES FOLDER "Neo")

