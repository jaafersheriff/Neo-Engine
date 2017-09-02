#!/usr/bin/bash

#g++ -std=c++11 -o $Neo.exe src/Main/* src/Context/* -I$GLM_INCLUDE_DIR -I$GLEW_DIR/include -I$GLFW_DIR/include -I$GLFW_DIR/debug -l$GLEW_DIRlib/Release/Win32/glew32s -L$GLFW_LIBRARIES -L$GLEW_DIR/lib/Release/Win32 
#-lopengl32.lib -lGLEW -lglfw -lGL -lGLU -lX11 -lpthread -lXxf86vm -lm -lglew32 -lSDL2

#Build
echo "Building..."
cd build
mingw32-make

# If successful compile
if [ $? -eq 0 ]; then
   echo "Running..."
   ./Neo.exe $@
else
	echo "FAIL"
fi

cd ..
