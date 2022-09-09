#include <n64/core/mmio/PIF.hpp>
#include <n64/core/Mem.hpp>
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
        channel++;
      }
    }
  }

  if(control & 8) {
    pifRam[63] &= ~8;
  }

  if(control & 48) {
    pifRam[63] = 128;
  }

  //pifRam[63] &= ~1;
}

}