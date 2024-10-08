#pragma once
#include <common.hpp>

static constexpr u16 rsqRom[] = {
  0xffff, 0xff00, 0xfe02, 0xfd06, 0xfc0b, 0xfb12, 0xfa1a, 0xf923, 0xf82e, 0xf73b, 0xf648, 0xf557, 0xf467, 0xf379,
  0xf28c, 0xf1a0, 0xf0b6, 0xefcd, 0xeee5, 0xedff, 0xed19, 0xec35, 0xeb52, 0xea71, 0xe990, 0xe8b1, 0xe7d3, 0xe6f6,
  0xe61b, 0xe540, 0xe467, 0xe38e, 0xe2b7, 0xe1e1, 0xe10d, 0xe039, 0xdf66, 0xde94, 0xddc4, 0xdcf4, 0xdc26, 0xdb59,
  0xda8c, 0xd9c1, 0xd8f7, 0xd82d, 0xd765, 0xd69e, 0xd5d7, 0xd512, 0xd44e, 0xd38a, 0xd2c8, 0xd206, 0xd146, 0xd086,
  0xcfc7, 0xcf0a, 0xce4d, 0xcd91, 0xccd6, 0xcc1b, 0xcb62, 0xcaa9, 0xc9f2, 0xc93b, 0xc885, 0xc7d0, 0xc71c, 0xc669,
  0xc5b6, 0xc504, 0xc453, 0xc3a3, 0xc2f4, 0xc245, 0xc198, 0xc0eb, 0xc03f, 0xbf93, 0xbee9, 0xbe3f, 0xbd96, 0xbced,
  0xbc46, 0xbb9f, 0xbaf8, 0xba53, 0xb9ae, 0xb90a, 0xb867, 0xb7c5, 0xb723, 0xb681, 0xb5e1, 0xb541, 0xb4a2, 0xb404,
  0xb366, 0xb2c9, 0xb22c, 0xb191, 0xb0f5, 0xb05b, 0xafc1, 0xaf28, 0xae8f, 0xadf7, 0xad60, 0xacc9, 0xac33, 0xab9e,
  0xab09, 0xaa75, 0xa9e1, 0xa94e, 0xa8bc, 0xa82a, 0xa799, 0xa708, 0xa678, 0xa5e8, 0xa559, 0xa4cb, 0xa43d, 0xa3b0,
  0xa323, 0xa297, 0xa20b, 0xa180, 0xa0f6, 0xa06c, 0x9fe2, 0x9f59, 0x9ed1, 0x9e49, 0x9dc2, 0x9d3b, 0x9cb4, 0x9c2f,
  0x9ba9, 0x9b25, 0x9aa0, 0x9a1c, 0x9999, 0x9916, 0x9894, 0x9812, 0x9791, 0x9710, 0x968f, 0x960f, 0x9590, 0x9511,
  0x9492, 0x9414, 0x9397, 0x931a, 0x929d, 0x9221, 0x91a5, 0x9129, 0x90af, 0x9034, 0x8fba, 0x8f40, 0x8ec7, 0x8e4f,
  0x8dd6, 0x8d5e, 0x8ce7, 0x8c70, 0x8bf9, 0x8b83, 0x8b0d, 0x8a98, 0x8a23, 0x89ae, 0x893a, 0x88c6, 0x8853, 0x87e0,
  0x876d, 0x86fb, 0x8689, 0x8618, 0x85a7, 0x8536, 0x84c6, 0x8456, 0x83e7, 0x8377, 0x8309, 0x829a, 0x822c, 0x81bf,
  0x8151, 0x80e4, 0x8078, 0x800c, 0x7fa0, 0x7f34, 0x7ec9, 0x7e5e, 0x7df4, 0x7d8a, 0x7d20, 0x7cb6, 0x7c4d, 0x7be5,
  0x7b7c, 0x7b14, 0x7aac, 0x7a45, 0x79de, 0x7977, 0x7911, 0x78ab, 0x7845, 0x77df, 0x777a, 0x7715, 0x76b1, 0x764d,
  0x75e9, 0x7585, 0x7522, 0x74bf, 0x745d, 0x73fa, 0x7398, 0x7337, 0x72d5, 0x7274, 0x7213, 0x71b3, 0x7152, 0x70f2,
  0x7093, 0x7033, 0x6fd4, 0x6f76, 0x6f17, 0x6eb9, 0x6e5b, 0x6dfd, 0x6da0, 0x6d43, 0x6ce6, 0x6c8a, 0x6c2d, 0x6bd1,
  0x6b76, 0x6b1a, 0x6abf, 0x6a64, 0x6a09, 0x6955, 0x68a1, 0x67ef, 0x673e, 0x668d, 0x65de, 0x6530, 0x6482, 0x63d6,
  0x632b, 0x6280, 0x61d7, 0x612e, 0x6087, 0x5fe0, 0x5f3a, 0x5e95, 0x5df1, 0x5d4e, 0x5cac, 0x5c0b, 0x5b6b, 0x5acb,
  0x5a2c, 0x598f, 0x58f2, 0x5855, 0x57ba, 0x5720, 0x5686, 0x55ed, 0x5555, 0x54be, 0x5427, 0x5391, 0x52fc, 0x5268,
  0x51d5, 0x5142, 0x50b0, 0x501f, 0x4f8e, 0x4efe, 0x4e6f, 0x4de1, 0x4d53, 0x4cc6, 0x4c3a, 0x4baf, 0x4b24, 0x4a9a,
  0x4a10, 0x4987, 0x48ff, 0x4878, 0x47f1, 0x476b, 0x46e5, 0x4660, 0x45dc, 0x4558, 0x44d5, 0x4453, 0x43d1, 0x434f,
  0x42cf, 0x424f, 0x41cf, 0x4151, 0x40d2, 0x4055, 0x3fd8, 0x3f5b, 0x3edf, 0x3e64, 0x3de9, 0x3d6e, 0x3cf5, 0x3c7c,
  0x3c03, 0x3b8b, 0x3b13, 0x3a9c, 0x3a26, 0x39b0, 0x393a, 0x38c5, 0x3851, 0x37dd, 0x3769, 0x36f6, 0x3684, 0x3612,
  0x35a0, 0x352f, 0x34bf, 0x344f, 0x33df, 0x3370, 0x3302, 0x3293, 0x3226, 0x31b9, 0x314c, 0x30df, 0x3074, 0x3008,
  0x2f9d, 0x2f33, 0x2ec8, 0x2e5f, 0x2df6, 0x2d8d, 0x2d24, 0x2cbc, 0x2c55, 0x2bee, 0x2b87, 0x2b21, 0x2abb, 0x2a55,
  0x29f0, 0x298b, 0x2927, 0x28c3, 0x2860, 0x27fd, 0x279a, 0x2738, 0x26d6, 0x2674, 0x2613, 0x25b2, 0x2552, 0x24f2,
  0x2492, 0x2432, 0x23d3, 0x2375, 0x2317, 0x22b9, 0x225b, 0x21fe, 0x21a1, 0x2145, 0x20e8, 0x208d, 0x2031, 0x1fd6,
  0x1f7b, 0x1f21, 0x1ec7, 0x1e6d, 0x1e13, 0x1dba, 0x1d61, 0x1d09, 0x1cb1, 0x1c59, 0x1c01, 0x1baa, 0x1b53, 0x1afc,
  0x1aa6, 0x1a50, 0x19fa, 0x19a5, 0x1950, 0x18fb, 0x18a7, 0x1853, 0x17ff, 0x17ab, 0x1758, 0x1705, 0x16b2, 0x1660,
  0x160d, 0x15bc, 0x156a, 0x1519, 0x14c8, 0x1477, 0x1426, 0x13d6, 0x1386, 0x1337, 0x12e7, 0x1298, 0x1249, 0x11fb,
  0x11ac, 0x115e, 0x1111, 0x10c3, 0x1076, 0x1029, 0x0fdc, 0x0f8f, 0x0f43, 0x0ef7, 0x0eab, 0x0e60, 0x0e15, 0x0dca,
  0x0d7f, 0x0d34, 0x0cea, 0x0ca0, 0x0c56, 0x0c0c, 0x0bc3, 0x0b7a, 0x0b31, 0x0ae8, 0x0aa0, 0x0a58, 0x0a10, 0x09c8,
  0x0981, 0x0939, 0x08f2, 0x08ab, 0x0865, 0x081e, 0x07d8, 0x0792, 0x074d, 0x0707, 0x06c2, 0x067d, 0x0638, 0x05f3,
  0x05af, 0x056a, 0x0526, 0x04e2, 0x049f, 0x045b, 0x0418, 0x03d5, 0x0392, 0x0350, 0x030d, 0x02cb, 0x0289, 0x0247,
  0x0206, 0x01c4, 0x0183, 0x0142, 0x0101, 0x00c0, 0x0080, 0x0040};
