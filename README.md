# Kaizen

[![CodeFactor](https://www.codefactor.io/repository/github/SimoneN64/Kaizen/badge/master)](https://www.codefactor.io/repository/github/SimoneN64/Kaizen/overview/master)
[![build](https://github.com/SimoneN64/Kaizen/actions/workflows/build.yml/badge.svg)](https://github.com/SimoneN64/Kaizen/actions/workflows/build.yml)

Rewrite of my Nintendo 64 emulator "[shibumi](https://github.com/SimoneN64/shibumi)".

![Mario's face](resources/mario.png?raw=true)

## Pre-built binaries
| Release                                                                                              |
|------------------------------------------------------------------------------------------------------|
| [Windows](https://nightly.link/SimoneN64/Gadolinium/workflows/build/master/gadolinium-windows.zip)   |
| [ Linux ](https://nightly.link/SimoneN64/Gadolinium/workflows/build/master/gadolinium-linux.zip)     |


## Build instructions:
First clone the repository: `git clone --recursive https://github.com/SimoneN64/Kaizen`

### Windows

This build uses Visual Studio with Vcpkg and Clang-cl

Dependencies:
- CMake 3.20 or higher
- SDL2 (install it by making sure that you're choosing the "vulkan" extension of the package and the x64-windows triplet: `vcpkg install sdl2[vulkan]:x64-windows`)
- fmtlib (install it by making sure that you're choosing the x64-windows triplet: `vcpkg install fmt:x64-windows`)
- nlohmann-json (install it by making sure that you're choosing the x64-windows triplet: `vcpkg install nlohmann-json:x64-windows`)

```
cd path/to/kaizen
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
cd path/to/kaizen
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release -S ../src
cmake --build . --config Release
```

## Running:
```
./path/to/kaizen [ROM] [Mupen Movie]
```

Your GPU needs to support Vulkan 1.1+, because the RDP is implemented via [ParaLLEl-RDP](https://github.com/Themaister/parallel-rdp).

## Roadmap
- [x] Full R4300i emulation
- [x] Full RCP emulation
- [x] Full TLB emulation
- [x] Full joybus emulation (but it's not configurable by the user at the moment)
- [ ] Qt or wxWidgets for native GUI (keeping ImGui as opt-in).
- [ ] JIT, with support for x86_64 and ARM (using an IR).
- [ ] Debug tools: disassembly, breakpoints, single-step and memory editor
- [ ] TAS tools
    - [x] TAS replay (using Mupen's format)
    - [x] Frame-advance
    - [ ] TAS input
    - [ ] Recording (using Mupen's format)
    - [ ] Save-states
    - [ ] Rewind
- [ ] Cheat support
- [ ] Allow to optionally pass a PIF image for the boot process (it's exclusively HLE'd at the moment)

This list will probably grow with time!

## Special thanks:

- [Dillonb](https://github.com/Dillonb) and [KieronJ](https://github.com/KieronJ) for bearing with me and my recurring brainfarts, and for the support :heart:
- [WhoBrokeTheBuild](https://github.com/WhoBrokeTheBuild) for the shader that allows letterboxing :rocket:
- [Kelpsy](https://github.com/Kelpsy), [fleroviux](https://github.com/fleroviux), [Kim-Dewelski](https://github.com/Kim-Dewelski), [Peach](https://github.com/wheremyfoodat/),
  [kivan](https://github.com/kivan117), [liuk](https://github.com/liuk7071) and [Skyler](https://github.com/skylersaleh) for the general support and motivation :heart:
- [Spec](https://github.com/spec-chum/) for help with testing on Windows, that helped form the final build instructions :heart:
