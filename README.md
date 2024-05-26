# Kaizen

[![CodeFactor](https://www.codefactor.io/repository/github/SimoneN64/Kaizen/badge/master)](https://www.codefactor.io/repository/github/SimoneN64/Kaizen/overview/master)
[![build](https://github.com/SimoneN64/Kaizen/actions/workflows/build.yml/badge.svg)](https://github.com/SimoneN64/Kaizen/actions/workflows/build.yml)

Rewrite of my Nintendo 64 emulator "[shibumi](https://github.com/SimoneN64/shibumi)".

![Mario's face](resources/mario.png?raw=true)

## Pre-built binaries
| Release                                                                                    |
|--------------------------------------------------------------------------------------------|
| [ Linux ](https://nightly.link/SimoneN64/Kaizen/workflows/build/master/kaizen-linux.zip)   |

## Socials
We have a [Discord server](https://discord.gg/htzNd2rRF6)

## Sister projects
- [**n64**](https://github.com/dillonb/n64): Low-level, fast, accurate and easy to use Nintendo 64 emulator
- [**Panda3DS**](https://github.com/wheremyfoodat/Panda3DS): A new HLE Nintendo 3DS emulator
- [**Dust**](https://github.com/kelpsyberry/dust): Nintendo DS emulator for desktop devices and the web, with debugging features and a focus on accuracy
- [**SkyEmu**](https://github.com/skylersaleh/SkyEmu): A low-level GameBoy, GameBoy Color, GameBoy Advance and Nintendo DS emulator that is designed to be easy to use, cross platform and accurate
- [**NanoBoyAdvance**](https://github.com/nba-emu/NanoBoyAdvance): A Game Boy Advance emulator focusing on hardware research and cycle-accurate emulation
- [**melonDS**](https://github.com/melonDS-emu/melonDS): "DS emulator, sorta"; a Nintendo DS emulator focused on accuracy and ease-of-use
- [**n64-emu**](https://github.com/kmc-jp/n64-emu): Experimental N64 emulator
- [**ares**](https://github.com/ares-emulator/ares): ares is a multi-system emulator that began development on October 14th, 2004. It focuses on accuracy and preservation.

## Build instructions:
First clone the repository: `git clone --recursive https://github.com/SimoneN64/Kaizen`

### Linux

Dependencies:
- GCC or Clang with C++17 support
- CMake 3.20 or higher
- SDL2
- Vulkan API (including the validation layers) + SPIR-V tools
- Qt6

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
- [x] TAS replay (using Mupen's format)
- [x] Qt for native GUI
- [ ] JIT, with support for x86_64 and ARM (using an IR).
- [ ] Debug tools: disassembly, breakpoints, single-step and memory editor
- [ ] TAS tools: TAS input, recording (using Mupen's format), save-states, rewind and frame-advance
- [ ] Cheat support
- [ ] Allow to optionally pass a PIF image for the boot process (it's HLE'd at the moment)
- [ ] Windows support when it stops being a pain in the rectum.

This list will probably grow with time!

## Special thanks:

- [Dillonb](https://github.com/Dillonb) and [KieronJ](https://github.com/KieronJ) for bearing with me and my recurring brainfarts, and for the support :heart:
- [WhoBrokeTheBuild](https://github.com/WhoBrokeTheBuild) for the shader that allows letterboxing :rocket:
- [Kelpsy](https://github.com/kelpsyberry), [fleroviux](https://github.com/fleroviux), [Kim-Dewelski](https://github.com/Kim-Dewelski), [Peach](https://github.com/wheremyfoodat/),
  [kivan](https://github.com/kivan117), [liuk](https://github.com/liuk7071) and [Skyler](https://github.com/skylersaleh) for the general support and motivation :heart:
- [Spec](https://github.com/spec-chum/) for being an awesome person in general :heart:

## Copyright

Nintendo 64 is a registered trademark of Nintendo Co., Ltd.
