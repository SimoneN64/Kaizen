#pragma once

#define RDRAM_SIZE 0x800000
#define RDRAM_DSIZE (RDRAM_SIZE - 1)
#define SRAM_SIZE 0x8000000
#define SRAM_DSIZE (SRAM_SIZE - 1)
#define DMEM_SIZE 0x1000
#define DMEM_DSIZE (DMEM_SIZE - 1)
#define IMEM_SIZE 0x1000
#define IMEM_DSIZE (IMEM_SIZE - 1)
#define PIF_RAM_SIZE 0x40
#define PIF_RAM_DSIZE (PIF_RAM_SIZE - 1)
#define PIF_BOOTROM_SIZE 0x7C0
#define PIF_BOOTROM_DSIZE (PIF_BOOTROM_SIZE - 1)

#define RDRAM_REGION 0 ... RDRAM_DSIZE
#define DMEM_REGION 0x04000000 ... DMEM_DSIZE
#define IMEM_REGION 0x04001000 ... IMEM_DSIZE
#define MMIO_REGION 0x04040000 ... 0x048FFFFF
#define SP_REGION 0x04040000 ... 0x040FFFFF
#define DP_CMD_REGION 0x04100000 ... 0x041FFFFF
#define MI_REGION 0x04300000 ... 0x043FFFFF
#define VI_REGION 0x04400000 ... 0x044FFFFF
#define AI_REGION 0x04500000 ... 0x045FFFFF
#define PI_REGION 0x04600000 ... 0x046FFFFF
#define RI_REGION 0x04700000 ... 0x047FFFFF
#define SI_REGION 0x04800000 ... 0x048FFFFF
#define SRAM_REGION 0x08000000 ... 0x0FFFFFFF
#define CART_REGION 0x10000000 ... 0x1FBFFFFF
#define PIF_ROM_REGION 0x1FC00000 ... 0x1FC007BF
#define PIF_RAM_REGION 0x1FC007C0 ... 0x1FC007FF