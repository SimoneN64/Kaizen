# CS_ARCH_AARCH64, 0, None

0x03,0x31,0x3c,0xd5 == mrs x3, HDFGRTR2_EL2
0x23,0x31,0x3c,0xd5 == mrs x3, HDFGWTR2_EL2
0x43,0x31,0x3c,0xd5 == mrs x3, HFGRTR2_EL2
0x63,0x31,0x3c,0xd5 == mrs x3, HFGWTR2_EL2
0xe3,0x31,0x3c,0xd5 == mrs x3, HFGITR2_EL2
0x03,0x31,0x1c,0xd5 == msr HDFGRTR2_EL2, x3
0x23,0x31,0x1c,0xd5 == msr HDFGWTR2_EL2, x3
0x43,0x31,0x1c,0xd5 == msr HFGRTR2_EL2, x3
0x63,0x31,0x1c,0xd5 == msr HFGWTR2_EL2, x3
0xe3,0x31,0x1c,0xd5 == msr HFGITR2_EL2, x3