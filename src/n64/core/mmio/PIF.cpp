#include <n64/core/mmio/PIF.hpp>
#include <n64/core/Mem.hpp>
#include <n64/core/cpu/Registers.hpp>
#include <util.hpp>

namespace n64 {
static int channel = 0;

void ProcessPIFCommands(u8* pifRam, Controller& controller, Mem& mem) {
  u8 control = pifRam[63];

  if(control & 1) {
    channel = 0;
    for(int i = 0; i < 63;) {
      u8* cmd = &pifRam[i++];
      u8 t = cmd[0] & 0x3f;

      if(t == 0 || t == 0x3D) {
        channel++;
      } else if (t == 0x3E) {
        break;
      } else if (t == 0x3F) {
        continue;
      } else {
        u8 r = pifRam[i++];
        r |= (1 << 7);
        if(r == 0xFE) {
          break;
        }

        u8 rlen = r & 0x3F;
        u8* res = &pifRam[i + t];
        switch(cmd[2]) {
          case 0xff:
            res[0] = 0x05;
            res[1] = 0x00;
            res[2] = 0x01;
            channel++;
            break;
          case 0:
            res[0] = 0x05;
            res[1] = 0x00;
            res[2] = 0x01;
            break;
          case 1:
            res[0] = controller.b1;
            res[1] = controller.b2;
            res[2] = controller.b3;
            res[3] = controller.b4;
            break;
          case 2: case 3: break;
          default: util::panic("Unimplemented PIF command {}", cmd[2]);
        }

        i += t + rlen;
      }
    }
  }

  if(control & 8) {
    pifRam[63] &= ~8;
  }

  if (control & 0x30) {
    pifRam[63] = 0x80;
  }
}

void DoPIFHLE(Mem& mem, Registers& regs, CartInfo cartInfo) {
  u32 cicType = cartInfo.cicType;
  bool pal = cartInfo.isPAL;
  mem.Write32<false>(regs, 0x1FC007E4, cicSeeds[cicType], regs.pc);

  switch(cicType) {
    case CIC_NUS_6101:
      mem.Write32<false>(regs, 0x318, RDRAM_SIZE, regs.pc);
      regs.gpr[2] = (s64)0xFFFFFFFFDF6445CC;
      regs.gpr[3] = (s64)0xFFFFFFFFDF6445CC;
      regs.gpr[4] = 0x45CC;
      regs.gpr[5] = 0x73EE317A;
      regs.gpr[6] = (s64)0xFFFFFFFFA4001F0C;
      regs.gpr[7] = (s64)0xFFFFFFFFA4001F08;
      regs.gpr[8] = 0xC0;
      regs.gpr[10] = 0x40;
      regs.gpr[11] = (s64)0xFFFFFFFFA4000040;
      regs.gpr[12] = (s64)0xFFFFFFFFC7601FAC;
      regs.gpr[13] = (s64)0xFFFFFFFFC7601FAC;
      regs.gpr[14] = (s64)0xFFFFFFFFB48E2ED6;
      regs.gpr[15] = (s64)0xFFFFFFFFBA1A7D4B;
      regs.gpr[20] = 0x0000000000000001;
      regs.gpr[22] = 0x000000000000003F;
      regs.gpr[23] = 0x0000000000000001;
      regs.gpr[24] = 0x0000000000000002;
      regs.gpr[25] = (s64)0xFFFFFFFF905F4718;
      regs.gpr[29] = (s64)0xFFFFFFFFA4001FF0;
      regs.gpr[31] = (s64)0xFFFFFFFFA4001550;
      regs.lo = (s64)0xFFFFFFFFBA1A7D4B;
      regs.hi = (s64)0xFFFFFFFF997EC317;
      break;
    case CIC_NUS_7102:
      mem.Write32<false>(regs, 0x318, RDRAM_SIZE, regs.pc);
      regs.gpr[1] = 0x0000000000000001;
      regs.gpr[2] = 0x000000001E324416;
      regs.gpr[3] = 0x000000001E324416;
      regs.gpr[4] = 0x0000000000004416;
      regs.gpr[5] = 0x000000000EC5D9AF;
      regs.gpr[6] = (s64)0xFFFFFFFFA4001F0C;
      regs.gpr[7] = (s64)0xFFFFFFFFA4001F08;
      regs.gpr[8] = 0x00000000000000C0;
      regs.gpr[10] = 0x0000000000000040;
      regs.gpr[11] = (s64)0xFFFFFFFFA4000040;
      regs.gpr[12] = 0x00000000495D3D7B;
      regs.gpr[13] = (s64)0xFFFFFFFF8B3DFA1E;
      regs.gpr[14] = 0x000000004798E4D4;
      regs.gpr[15] = (s64)0xFFFFFFFFF1D30682;
      regs.gpr[22] = 0x000000000000003F;
      regs.gpr[23] = 0x0000000000000007;
      regs.gpr[25] = 0x0000000013D05CAB;
      regs.gpr[29] = (s64)0xFFFFFFFFA4001FF0;
      regs.gpr[31] = (s64)0xFFFFFFFFA4001554;

      regs.lo = (s64)0xFFFFFFFFF1D30682;
      regs.hi = 0x0000000010054A98;
      break;
    case CIC_NUS_6102_7101:
      mem.Write32<false>(regs, 0x318, RDRAM_SIZE, regs.pc);
      regs.gpr[1] = 0x0000000000000001;
      regs.gpr[2] = 0x000000000EBDA536;
      regs.gpr[3] = 0x000000000EBDA536;
      regs.gpr[4] = 0x000000000000A536;
      regs.gpr[5] = (s64)0xFFFFFFFFC0F1D859;
      regs.gpr[6] = (s64)0xFFFFFFFFA4001F0C;
      regs.gpr[7] = (s64)0xFFFFFFFFA4001F08;
      regs.gpr[8] = 0x00000000000000C0;
      regs.gpr[10] = 0x0000000000000040;
      regs.gpr[11] = (s64)0xFFFFFFFFA4000040;
      regs.gpr[12] = (s64)0xFFFFFFFFED10D0B3;
      regs.gpr[13] = 0x000000001402A4CC;
      regs.gpr[14] = 0x000000002DE108EA;
      regs.gpr[15] = 0x000000003103E121;
      regs.gpr[20] = 0x0000000000000001;
      regs.gpr[25] = (s64)0xFFFFFFFF9DEBB54F;
      regs.gpr[29] = (s64)0xFFFFFFFFA4001FF0;
      regs.gpr[31] = (s64)0xFFFFFFFFA4001550;

      regs.hi = 0x000000003FC18657;
      regs.lo = 0x000000003103E121;

      if (pal) {
        regs.gpr[20] = 0x0000000000000000;
        regs.gpr[23] = 0x0000000000000006;
        regs.gpr[31] = (s64)0xFFFFFFFFA4001554;
      }
      break;
    case CIC_NUS_6103_7103:
      mem.Write32<false>(regs, 0x318, RDRAM_SIZE, regs.pc);
      regs.gpr[0] = 0x0000000000000000;
      regs.gpr[1] = 0x0000000000000001;
      regs.gpr[2] = 0x0000000049A5EE96;
      regs.gpr[3] = 0x0000000049A5EE96;
      regs.gpr[4] = 0x000000000000EE96;
      regs.gpr[5] = (s64)0xFFFFFFFFD4646273;
      regs.gpr[6] = (s64)0xFFFFFFFFA4001F0C;
      regs.gpr[7] = (s64)0xFFFFFFFFA4001F08;
      regs.gpr[8] = 0x00000000000000C0;
      regs.gpr[9] = 0x0000000000000000;
      regs.gpr[10] = 0x0000000000000040;
      regs.gpr[11] = (s64)0xFFFFFFFFA4000040;
      regs.gpr[12] = (s64)0xFFFFFFFFCE9DFBF7;
      regs.gpr[13] = (s64)0xFFFFFFFFCE9DFBF7;
      regs.gpr[14] = 0x000000001AF99984;
      regs.gpr[15] = 0x0000000018B63D28;
      regs.gpr[16] = 0x0000000000000000;
      regs.gpr[17] = 0x0000000000000000;
      regs.gpr[18] = 0x0000000000000000;
      regs.gpr[19] = 0x0000000000000000;
      regs.gpr[20] = 0x0000000000000001;
      regs.gpr[21] = 0x0000000000000000;
      regs.gpr[23] = 0x0000000000000000;
      regs.gpr[24] = 0x0000000000000000;
      regs.gpr[25] = (s64)0xFFFFFFFF825B21C9;
      regs.gpr[26] = 0x0000000000000000;
      regs.gpr[27] = 0x0000000000000000;
      regs.gpr[28] = 0x0000000000000000;
      regs.gpr[29] = (s64)0xFFFFFFFFA4001FF0;
      regs.gpr[30] = 0x0000000000000000;
      regs.gpr[31] = (s64)0xFFFFFFFFA4001550;

      regs.lo = 0x0000000018B63D28;
      regs.hi = 0x00000000625C2BBE;

      if (pal) {
        regs.gpr[20] = 0x0000000000000000;
        regs.gpr[23] = 0x0000000000000006;
        regs.gpr[31] = (s64)0xFFFFFFFFA4001554;
      }
      break;
    case CIC_NUS_6105_7105:
      mem.Write32<false>(regs, 0x3F0, RDRAM_SIZE, regs.pc);
      regs.gpr[2] = (s64)0xFFFFFFFFF58B0FBF;
      regs.gpr[3] = (s64)0xFFFFFFFFF58B0FBF;
      regs.gpr[4] = 0x0000000000000FBF;
      regs.gpr[5] = (s64)0xFFFFFFFFDECAAAD1;
      regs.gpr[6] = (s64)0xFFFFFFFFA4001F0C;
      regs.gpr[7] = (s64)0xFFFFFFFFA4001F08;
      regs.gpr[8] = 0x00000000000000C0;
      regs.gpr[10] = 0x0000000000000040;
      regs.gpr[11] = (s64)0xFFFFFFFFA4000040;
      regs.gpr[12] = (s64)0xFFFFFFFF9651F81E;
      regs.gpr[13] = 0x000000002D42AAC5;
      regs.gpr[14] = 0x00000000489B52CF;
      regs.gpr[15] = 0x0000000056584D60;
      regs.gpr[20] = 0x0000000000000001;
      regs.gpr[24] = 0x0000000000000002;
      regs.gpr[25] = (s64)0xFFFFFFFFCDCE565F;
      regs.gpr[29] = (s64)0xFFFFFFFFA4001FF0;
      regs.gpr[31] = (s64)0xFFFFFFFFA4001550;

      regs.lo = 0x0000000056584D60;
      regs.hi = 0x000000004BE35D1F;

      if (pal) {
        regs.gpr[20] = 0x0000000000000000;
        regs.gpr[23] = 0x0000000000000006;
        regs.gpr[31] = (s64)0xFFFFFFFFA4001554;
      }

      mem.Write32<false>(regs, 0x04001000, 0x3C0DBFC0, regs.pc);
      mem.Write32<false>(regs, 0x04001004, 0x8DA807FC, regs.pc);
      mem.Write32<false>(regs, 0x04001008, 0x25AD07C0, regs.pc);
      mem.Write32<false>(regs, 0x0400100C, 0x31080080, regs.pc);
      mem.Write32<false>(regs, 0x04001000, 0x5500FFFC, regs.pc);
      mem.Write32<false>(regs, 0x04001004, 0x3C0DBFC0, regs.pc);
      mem.Write32<false>(regs, 0x04001008, 0x8DA80024, regs.pc);
      mem.Write32<false>(regs, 0x0400100C, 0x3C0BB000, regs.pc);
      break;
    case CIC_NUS_6106_7106:
      regs.gpr[2] = (s64)0xFFFFFFFFA95930A4;
      regs.gpr[3] = (s64)0xFFFFFFFFA95930A4;
      regs.gpr[4] = 0x00000000000030A4;
      regs.gpr[5] = (s64)0xFFFFFFFFB04DC903;
      regs.gpr[6] = (s64)0xFFFFFFFFA4001F0C;
      regs.gpr[7] = (s64)0xFFFFFFFFA4001F08;
      regs.gpr[8] = 0x00000000000000C0;
      regs.gpr[10] = 0x0000000000000040;
      regs.gpr[11] = (s64)0xFFFFFFFFA4000040;
      regs.gpr[12] = (s64)0xFFFFFFFFBCB59510;
      regs.gpr[13] = (s64)0xFFFFFFFFBCB59510;
      regs.gpr[14] = 0x000000000CF85C13;
      regs.gpr[15] = 0x000000007A3C07F4;
      regs.gpr[20] = 0x0000000000000001;
      regs.gpr[24] = 0x0000000000000002;
      regs.gpr[25] = 0x00000000465E3F72;
      regs.gpr[29] = (s64)0xFFFFFFFFA4001FF0;
      regs.gpr[30] = 0x0000000000000000;
      regs.gpr[31] = (s64)0xFFFFFFFFA4001550;

      regs.lo = 0x000000007A3C07F4;
      regs.hi = 0x0000000023953898;

      if (pal) {
        regs.gpr[20] = 0x0000000000000000;
        regs.gpr[23] = 0x0000000000000006;
        regs.gpr[31] = (s64)0xFFFFFFFFA4001554;
      }
      break;
  }

  regs.gpr[22] = (cicSeeds[cicType] >> 8) & 0xFF;
  regs.cop0.Reset();
}

}