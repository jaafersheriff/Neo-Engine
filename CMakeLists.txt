cmake_minimum_required(VERSION 3.17)

set(WIN_SDK_VERSION 10.0.19041.0)
set(WIN_SDK_MIN_VERSION 10.0.17134.0)
set(CMAKE_SYSTEM_VERSION ${WIN_SDK_VERSION})

set_property(GLOBAL PROPERTY USE_FOLDERS ON) # enables folder structures in IDEs
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(Neo CXX C)

# disable warnings on different compilers for our external dependencies (aka, source not written or maintained by us)
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	set(disableWarning /W0)
	add_compile_options("/WX" "/W4")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ignore:4075,4264,4201")
else()
	message(FATAL_ERROR	"Unsupported compiler: ${CMAKE_CXX_COMPILER_ID}")
endif()

if(MSVC)
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /ZI")
	set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /ZI")
	set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /SAFESEH:NO")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SAFESEH:NO")
	set(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} /INCREMENTAL /EDITANDCONTINUE")
	set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /INCREMENTAL /EDITANDCONTINUE")
	set(CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO "${CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO} /INCREMENTAL /EDITANDCONTINUE")
	set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO} /INCREMENTAL /EDITANDCONTINUE")
endif()

if(MSVC)
	add_compile_options(
		/FC
		/MP
		/Zc:rvalueCast
	)
	add_compile_definitions(
		_CRT_SECURE_NO_WARNINGS
		NOMINMAX
		WIN32_LEAN_AND_MEAN
)

endif()

add_compile_definitions(
	$<$<CONFIG:Debug>:DEBUG_MODE>
	$<$<CONFIG:RelWithDebInfo>:DEBUG_MODE>
)

#--------------------------------------------------------------------
# Hide the console window in visual studio projects - Release
#--------------------------------------------------------------------
if(MSVC)
	set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
endif()

set(CPACK_COMPONENTS_GROUPING IGNORE)
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY 0)

include(cmake/InitLib.cmake)
include(cmake/pch.cmake)

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/Engine")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/Demos")
