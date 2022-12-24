# Gadolinium

[![CodeFactor](https://www.codefactor.io/repository/github/cocosimone/Gadolinium/badge/master)](https://www.codefactor.io/repository/github/cocosimone/Gadolinium/overview/master)
[![build](https://github.com/CocoSimone/Gadolinium/actions/workflows/build.yml/badge.svg)](https://github.com/CocoSimone/Gadolinium/actions/workflows/build.yml)

Rewrite of my Nintendo 64 emulator "[shibumi](https://github.com/CocoSimone/shibumi)".

![Mario's face](resources/mario.png?raw=true)

## Pre-built binaries
| Release                                                                                                       |
|---------------------------------------------------------------------------------------------------------------|
| [Windows (Release)](https://nightly.link/CocoSimone/Gadolinium/workflows/build/master/gadolinium-windows.zip) |
| [Linux   (Release)](https://nightly.link/CocoSimone/Gadolinium/workflows/build/master/gadolinium-linux.zip)   |


## Build instructions:
First clone the repository: `git clone --recursive https://github.com/CocoSimone/Gadolinium`

### Windows

This build uses Visual Studio with Vcpkg and Clang-cl

Dependencies:
- CMake 3.20 or higher
- SDL2 (install it by making sure that you're choosing the "vulkan" extension of the package and the x64-windows triplet: `vcpkg install sdl2[vulkan]:x64-windows`)
- fmtlib (install it by making sure that you're choosing the x64-windows triplet: `vcpkg install fmt:x64-windows`)
- nlohmann-json (install it by making sure that you're choosing the x64-windows triplet: `vcpkg install nlohmann-json:x64-windows`)

```
cd path/to/gadolinium
mkdir build
cd build
cmake -T clangcl -DCMAKE_TOOLCHAIN_FILE=path/to/vcpkg/scripts/buildsystems/vcpkg.cmake -DCMAKE_BUILD_TYPE=Release -S ../src
cmake --build . --config Release
```

### Linux

Dependencies:
- GCC or Clang with C++17 support
- CMake 3.20 or higher
- SDL2
- fmtlib
- Vulkan API (including the validation layers) + SPIR-V tools
- nlohmann-json

```
cd path/to/gadolinium
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release -S ../src
cmake --build . --config Release
```

Special thanks:

- [Dillonb](https://github.com/Dillonb) and [KieronJ](https://github.com/KieronJ) for bearing with me and my recurring brainfarts, and for the support :heart:
- [WhoBrokeTheBuild](https://github.com/WhoBrokeTheBuild) for the shader that allows letterboxing :rocket:
- [Kelpsy](https://github.com/Kelpsy), [fleroviux](https://github.com/fleroviux), [Kim-Dewelski](https://github.com/Kim-Dewelski), [Peach](https://github.com/wheremyfoodat/),
  [kivan](https://github.com/kivan117), [liuk](https://github.com/liuk7071) and [Skyler](https://github.com/skylersaleh) for the general support and motivation :heart:
- [Spec](https://github.com/spec-chum/) for help with testing on Windows, that helped form the final build instructions :heart:
