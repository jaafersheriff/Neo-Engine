function(add_precompiled_header _target _input)
	target_precompile_headers(
		${_target}
		PRIVATE
		"$<$<COMPILE_LANGUAGE:CXX>:${_input}>"
	)
endfunction()

