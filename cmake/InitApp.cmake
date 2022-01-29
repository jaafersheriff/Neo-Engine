
function (InitApp TargetID) 
	message(STATUS "Generating App/${TargetID}")

	project(${TargetID})
	add_library(${TargetID})

	include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
	
	file(GLOB_RECURSE CPP_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.*)
	file(GLOB_RECURSE HPP_FILES ${CMAKE_CURRENT_SOURCE_DIR}/include/*.*)
	targetCaptureFiles("${CPP_FILES}" "${HPP_FILES}" "${TargetID}")

	target_sources(${TargetID}
	PRIVATE
		${CPP_FILES}
		${HPP_FILES}
	)

	target_include_directories(${TargetID}
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}/include
	PRIVATE
		"include/${TargetID}"
	)

	set_target_properties(${TargetID} PROPERTIES FOLDER "Neo")

endfunction()


