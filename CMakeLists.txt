cmake_minimum_required(VERSION 2.7)

# Name of the project
project(Neo)

# Use glob to get the list of all source files.
file(GLOB_RECURSE SOURCES "src/*.cpp")

#Set up link targets
if(MINGWW) 
    set(GLEW_DIR "$ENV{GLEW_DIR_MINGW}")
else()
    set(GLEW_DIR "$ENV{GLEW_DIR}")
endif()
if(NOT GLEW_DIR)
  message(FATAL_ERROR "Please point the environment variable GLEW_DIR to the root directory of your GLEW installation.")
else()
  message(${GLEW_DIR})
endif()

if(WIN32 AND NOT MINGW)
  # With prebuilt binaries
  link_directories(${GLEW_DIR}/lib/Release/Win32)
endif()

# We don't really need to include header and resource files to build, but it's
# nice to have them show up in IDEs.
file(GLOB_RECURSE HEADERS "src/*.h")
file(GLOB_RECURSE HEADERS "src/*.hpp")
file(GLOB_RECURSE GLSL "resources/*.glsl")

# Set the executable.
add_executable(${CMAKE_PROJECT_NAME} ${SOURCES} ${HEADERS} ${GLSL})

# Get the GLM environment variable. Since GLM is a header-only library, we
# just need to add it to the include directory.
set(GLM_INCLUDE_DIR "$ENV{GLM_INCLUDE_DIR}")
if(NOT GLM_INCLUDE_DIR)
  MESSAGE(FATAL_ERROR "Please point the environment variable GLM_INCLUDE_DIR to the include directory of your GLM installation.")
else()
  message(${GLM_INCLUDE_DIR})
endif()
include_directories(${GLM_INCLUDE_DIR})

# Get the GLFW environment variable. There should be a CMakeLists.txt in the 
# specified directory.
set(GLFW_DIR "$ENV{GLFW_DIR}")
if(NOT GLFW_DIR)
  message(FATAL_ERROR "Please point the environment variable GLFW_DIR to the root directory of your GLFW3 installation.")
else()
   message(${GLFW_DIR})
endif()
option(GLFW_BUILD_EXAMPLES "GLFW_BUILD_EXAMPLES" OFF)
option(GLFW_BUILD_TESTS "GLFW_BUILD_TESTS" OFF)
option(GLFW_BUILD_DOCS "GLFW_BUILD_DOCS" OFF)
if(CMAKE_BUILD_TYPE MATCHES Release)
  add_subdirectory(${GLFW_DIR} ${GLFW_DIR}/release)
else()
  add_subdirectory(${GLFW_DIR} ${GLFW_DIR}/debug)
endif()
include_directories(${GLFW_DIR}/include)
target_link_libraries(${CMAKE_PROJECT_NAME} glfw ${GLFW_LIBRARIES})

# Get the GLEW environment variable.
include_directories(${GLEW_DIR}/include)
if(WIN32)
  if(MINGW)
    target_link_libraries(${CMAKE_PROJECT_NAME} C:/MinGW/msys/1.0/lib/glew-2.1.0/lib/glew32.dll)
  else()
    target_link_libraries(${CMAKE_PROJECT_NAME} ${GLEW_DIR}/lib/Release/Win32/glew32s.lib)
  endif()
else()
  target_link_libraries(${CMAKE_PROJECT_NAME} ${GLEW_DIR}/lib/libGLEW.a)
endif()

# OS specific options and libraries
if(WIN32)
  # c++0x is enabled by default.
  # -Wall produces way too many warnings.
  # -pedantic is not supported.
  target_link_libraries(${CMAKE_PROJECT_NAME} opengl32.lib)
else()
  # Enable all pedantic warnings.
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -Wall -pedantic")
  if(APPLE)
    # Add required frameworks for GLFW.
    target_link_libraries(${CMAKE_PROJECT_NAME} "-framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo")
  else()
    #Link the Linux OpenGL library
    target_link_libraries(${CMAKE_PROJECT_NAME} "GL")
  endif()
endif()

include_directories(${PROJECT_SOURCE_DIR}/src)

# Set C++11
set (CMAKE_CXX_STANDARD 11)
