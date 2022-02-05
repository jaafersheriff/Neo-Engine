function(InitLib) 
	_GetTargetName(TargetID)
	_InitLibWithName("${TargetID}")
endfunction()

function(_GetTargetName TargetID)
	get_filename_component(_TargetID ${CMAKE_CURRENT_SOURCE_DIR} NAME)
	set("${TargetID}" "${_TargetID}" PARENT_SCOPE)
endfunction()

function (_InitLibWithName TargetID) 
	message(STATUS "Generating Neo/${TargetID}")

	project(${TargetID})
	add_library(${TargetID})

	target_include_directories(${TargetID} 
	PUBLIC
		${CMAKE_SOURCE_DIR}/Engine
	PRIVATE
	)
	
	file(GLOB_RECURSE CPP_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
	file(GLOB_RECURSE HPP_FILES 
			  ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp
			  ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
	source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES 
		${CPP_FILES} 
		${HPP_FILES})
	target_sources(${TargetID}
	PRIVATE
		${CPP_FILES}
		${HPP_FILES}
	)

	set_target_properties(${TargetID} PROPERTIES FOLDER "Neo")

endfunction()


