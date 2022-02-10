
function (InitApp TargetID) 
	message(STATUS "Generating App/${TargetID}")

	project(${TargetID})
	add_executable(${TargetID})

	include_directories(${CMAKE_CURRENT_SOURCE_DIR}/${TargetID})
	
	file(GLOB_RECURSE CPP_FILES ${CMAKE_CURRENT_SOURCE_DIR}/${TargetID}/*.cpp)
	file(GLOB_RECURSE HPP_FILES 
			  ${CMAKE_CURRENT_SOURCE_DIR}/${TargetID}/*.hpp
			  ${CMAKE_CURRENT_SOURCE_DIR}/${TargetID}/*.h
	)

	target_sources(${TargetID}
	PRIVATE
		${CPP_FILES}
		${HPP_FILES}
	)
	source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/${TargetID} FILES 
		${CPP_FILES} 
		${HPP_FILES})

	target_include_directories(${TargetID}
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}/${TargetID}
	PRIVATE
	)

	target_link_libraries(${TargetID}
	PUBLIC
	PRIVATE
		Neo
	)

	set_target_properties(${TargetID} PROPERTIES FOLDER "Apps")

	set_property(TARGET ${TargetID} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/Apps/${TargetID}")

	# Assume res/ and shaders/
	# file(GLOB_RECURSE RESOURCES ${CMAKE_CURRENT_SOURCE_DIR}/${TargetID}/res/*)
	# source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/${TargetID}" FILES ${RESOURCES})
	# set_property(TARGET ${TargetID} APPEND PROPERTY SOURCES ${RESOURCES})

	file(GLOB_RECURSE SHADERS ${CMAKE_CURRENT_SOURCE_DIR}/${TargetID}/shaders/*)
	source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/${TargetID}" FILES ${SHADERS})
	set_property(TARGET ${TargetID} APPEND PROPERTY SOURCES ${SHADERS})

endfunction()


