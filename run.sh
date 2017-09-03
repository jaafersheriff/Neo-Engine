#!/usr/bin/bash

echo "Building..."
cd build

cmake .. -G 'MinGW Makefiles'
if [ $? -ne 0 ]; then
   echo "Failed cmake"
   cd ..
   exit 1
fi

mingw32-make
if [ $? -ne 0 ]; then
   echo "Failed make"
   cd ..
   exit 1
fi

echo "Running.."
./Neo.exe $@
cd ..
