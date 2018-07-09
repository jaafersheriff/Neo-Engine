#!/usr/bin/bash 

echo "Building..."
cd build

cmake .. -G 'MSYS Makefiles'
if [ $? -eq 0 ]; then
   make
   if [ $? -eq 0 ]; then
      echo "Running..."; echo
      ./Neo.exe $@
   else
      echo "Failed make"
   fi
else 
   echo "Failed cmake"
fi

cd ..

