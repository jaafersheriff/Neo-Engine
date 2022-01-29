function(groupSources SRCS TargetID) 
	foreach(source ${SRCS}) 
		# Get parent dir
		get_filename_component(source_path "${source}" DIRECTORY)

		string(REGEX REPLACE ".*/Neo-Engine/Engine" "" source_path_base "${source_path}")
		string(REGEX REPLACE "${TargetID}[/]" "" source_path_base "${source_path_base}")
		string(REGEX REPLACE "[/]${TargetID}" "" source_path_base "${source_path_base}")

		string(REPLACE "/" "\\" source_path_msvc "${source_path_base}")

		source_group("${source_path_msvc}" FILES ${source})
	endforeach()
endfunction() 

function(captureFiles CPP_FILES HPP_FILES)
	GetTargetName(TargetID)
	targetCaptureFiles("${CPP_FILES}" "${HPP_FILES}" "${TargetID}")
endfunction()

function(targetCaptureFiles CPP_FILES HPP_FILES TargetID)
	groupSources("${CPP_FILES}" "${TargetID}")
	groupSources("${HPP_FILES}" "${TargetID}")
endfunction()


