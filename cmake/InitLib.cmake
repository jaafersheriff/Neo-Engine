function(GetTargetName TargetID)
	get_filename_component(_TargetID ${CMAKE_CURRENT_SOURCE_DIR} NAME)
	set("${TargetID}" "${_TargetID}" PARENT_SCOPE)
endfunction()

function(InitLib) 
	GetTargetName(TargetID)
	InitLibWithName("${TargetID}")
endfunction()

function (InitLibWithName TargetID) 
	message(STATUS "Generating Neo/${TargetID}")

	project(${TargetID})
	add_library(${TargetID})

	include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
	
	file(GLOB_RECURSE CPP_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
	file(GLOB_RECURSE HPP_FILES 
			  ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp
			  ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
	targetCaptureFiles("${CPP_FILES}" "${HPP_FILES}" "${TargetID}")

	target_sources(${TargetID}
	PRIVATE
		${CPP_FILES}
		${HPP_FILES}
	)

	target_include_directories(${TargetID}
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}
	PRIVATE
		"include/${TargetID}"
	)

	set_target_properties(${TargetID} PROPERTIES FOLDER "Neo")

endfunction()


