# Changelog

This file always reflects the *latest* changes up until a release. Then it gets wiped out and updated again.

## Changes

New features:
* Debugger with memory view, registers view and code disassembly with stepping and breakpoints
* Customizable controller mappings

Fixes:
* Screen no longer black when the game is paused
* Fix crashes when closing the emulator while a game is running
* Other minor fixes such as:
    - Remove `nativefiledialog-extended` submodule as it is no longer needed
    - Small CMake refactoring
    - Correct RDRAM masking in RSP

Updates:
* Update SDL to version 3
* Update `parallel-rdp`
* Update `xbyak` subtree