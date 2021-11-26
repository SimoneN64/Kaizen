#pragma once
#include <stdint.h>
#include <stddef.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;

#define PACKED __attribute__((__packed__))
#define INLINE static inline __attribute__((always_inline))

#define GB_ASPECT_RATIO (float)10 / 9
#define BOOTROM_SIZE 0x100
#define BOOTROM_DSIZE (BOOTROM_SIZE - 1)
#define EXTRAM_SIZE 0x2000
#define EXTRAM_DSIZE (EXTRAM_SIZE - 1)
#define WRAM_SIZE 0x2000
#define WRAM_DSIZE (WRAM_SIZE - 1)
#define HRAM_SIZE 0x7f
#define HRAM_DSIZE (HRAM_SIZE - 1)
#define ROM_SIZE_MIN 0x8000
#define ROM_DSIZE_MIN (ROM_SIZE_MIN - 1)
#ifndef EMU_DIR
#define EMU_DIR ""
#endif