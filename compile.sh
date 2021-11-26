#!/bin/bash
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
  if [ -z "$1" ]; then
    cmake -DCMAKE_BUILD_TYPE=Release -B build-Release
    make -j8 -C build-Release
    cp build-Release/compile_commands.json .
  else
    cmake -DCMAKE_BUILD_TYPE=$1 -B build-$1
    make -j8 -C build-$1
    cp build-$1/compile_commands.json .
  fi
elif [[ "$OSTYPE" == "msys" ]]; then
  if [ -z "$1" ]; then
    cmake -G"Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=Clang -DCMAKE_C_COMPILER=Clang -B build-windows-Release
    make -j8 -C build-windows-Release
    cp build-windows-Release/compile_commands.json .
  else
    cmake -G"Unix Makefiles" -DCMAKE_BUILD_TYPE=$1 -DCMAKE_CXX_COMPILER=Clang -DCMAKE_C_COMPILER=Clang -B build-windows-$1
    make -j8 -C build-windows-$1
    cp build-windows-$1/compile_commands.json .
  fi
fi