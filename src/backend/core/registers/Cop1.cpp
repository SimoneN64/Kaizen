#include <core/Interpreter.hpp>
#include <core/registers/Cop1.hpp>
#include <core/registers/Registers.hpp>
#include <log.hpp>

namespace n64 {
Cop1::Cop1(Registers &regs) : regs(regs) { Reset(); }

void Cop1::Reset() {
  fcr0 = 0xa00;
  fcr31.write(0x01000800);
  memset(fgr, 0, 32 * sizeof(FloatingPointReg));
}

template <class T>
void Cop1::decode(T &cpu, u32 instr) {
  if constexpr (std::is_same_v<decltype(cpu), Interpreter &>) {
    decodeInterp(cpu, instr);
  } else {
    Util::panic("What the fuck did you just give me?!");
  }
}

template void Cop1::decode<Interpreter>(Interpreter &, u32);
template void Cop1::decode<JIT>(JIT &, u32);

void Cop1::decodeInterp(Interpreter &cpu, u32 instr) {

  u8 mask_sub = (instr >> 21) & 0x1F;
  u8 mask_fun = instr & 0x3F;
  u8 mask_branch = (instr >> 16) & 0x1F;
  switch (mask_sub) {
  // 000r_rccc
  case 0x00:
    mfc1(instr);
    break;
  case 0x01:
    dmfc1(instr);
    break;
  case 0x02:
    cfc1(instr);
    break;
  case 0x03:
    unimplemented();
    break;
  case 0x04:
    mtc1(instr);
    break;
  case 0x05:
    dmtc1(instr);
    break;
  case 0x06:
    ctc1(instr);
    break;
  case 0x07:
    unimplemented();
    break;
  case 0x08:
    switch (mask_branch) {
    case 0:
      if (!CheckFPUUsable())
        return;
      cpu.b(instr, !fcr31.compare);
      break;
    case 1:
      if (!CheckFPUUsable())
        return;
      cpu.b(instr, fcr31.compare);
      break;
    case 2:
      if (!CheckFPUUsable())
        return;
      cpu.bl(instr, !fcr31.compare);
      break;
    case 3:
      if (!CheckFPUUsable())
        return;
      cpu.bl(instr, fcr31.compare);
      break;
    default:
      Util::panic("Undefined BC COP1 {:02X}", mask_branch);
    }
    break;
  case 0x10: // s
    switch (mask_fun) {
    case 0x00:
      adds(instr);
      break;
    case 0x01:
      subs(instr);
      break;
    case 0x02:
      muls(instr);
      break;
    case 0x03:
      divs(instr);
      break;
    case 0x04:
      sqrts(instr);
      break;
    case 0x05:
      abss(instr);
      break;
    case 0x06:
      movs(instr);
      break;
    case 0x07:
      negs(instr);
      break;
    case 0x08:
      roundls(instr);
      break;
    case 0x09:
      truncls(instr);
      break;
    case 0x0A:
      ceills(instr);
      break;
    case 0x0B:
      floorls(instr);
      break;
    case 0x0C:
      roundws(instr);
      break;
    case 0x0D:
      truncws(instr);
      break;
    case 0x0E:
      ceilws(instr);
      break;
    case 0x0F:
      floorws(instr);
      break;
    case 0x21:
      cvtds(instr);
      break;
    case 0x24:
      cvtws(instr);
      break;
    case 0x25:
      cvtls(instr);
      break;
    case 0x30:
      cf<float>(instr);
      break;
    case 0x31:
      cun<float>(instr);
      break;
    case 0x32:
      ceq<float>(instr);
      break;
    case 0x33:
      cueq<float>(instr);
      break;
    case 0x34:
      colt<float>(instr);
      break;
    case 0x35:
      cult<float>(instr);
      break;
    case 0x36:
      cole<float>(instr);
      break;
    case 0x37:
      cule<float>(instr);
      break;
    case 0x38:
      csf<float>(instr);
      break;
    case 0x39:
      cngle<float>(instr);
      break;
    case 0x3A:
      cseq<float>(instr);
      break;
    case 0x3B:
      cngl<float>(instr);
      break;
    case 0x3C:
      clt<float>(instr);
      break;
    case 0x3D:
      cnge<float>(instr);
      break;
    case 0x3E:
      cle<float>(instr);
      break;
    case 0x3F:
      cngt<float>(instr);
      break;
    default:
      unimplemented();
    }
    break;
  case 0x11: // d
    switch (mask_fun) {
    case 0x00:
      addd(instr);
      break;
    case 0x01:
      subd(instr);
      break;
    case 0x02:
      muld(instr);
      break;
    case 0x03:
      divd(instr);
      break;
    case 0x04:
      sqrtd(instr);
      break;
    case 0x05:
      absd(instr);
      break;
    case 0x06:
      movd(instr);
      break;
    case 0x07:
      negd(instr);
      break;
    case 0x08:
      roundld(instr);
      break;
    case 0x09:
      truncld(instr);
      break;
    case 0x0A:
      ceilld(instr);
      break;
    case 0x0B:
      floorld(instr);
      break;
    case 0x0C:
      roundwd(instr);
      break;
    case 0x0D:
      truncwd(instr);
      break;
    case 0x0E:
      ceilwd(instr);
      break;
    case 0x0F:
      floorwd(instr);
      break;
    case 0x20:
      cvtsd(instr);
      break;
    case 0x24:
      cvtwd(instr);
      break;
    case 0x25:
      cvtld(instr);
      break;
    case 0x30:
      cf<double>(instr);
      break;
    case 0x31:
      cun<double>(instr);
      break;
    case 0x32:
      ceq<double>(instr);
      break;
    case 0x33:
      cueq<double>(instr);
      break;
    case 0x34:
      colt<double>(instr);
      break;
    case 0x35:
      cult<double>(instr);
      break;
    case 0x36:
      cole<double>(instr);
      break;
    case 0x37:
      cule<double>(instr);
      break;
    case 0x38:
      csf<double>(instr);
      break;
    case 0x39:
      cngle<double>(instr);
      break;
    case 0x3A:
      cseq<double>(instr);
      break;
    case 0x3B:
      cngl<double>(instr);
      break;
    case 0x3C:
      clt<double>(instr);
      break;
    case 0x3D:
      cnge<double>(instr);
      break;
    case 0x3E:
      cle<double>(instr);
      break;
    case 0x3F:
      cngt<double>(instr);
      break;
    default:
      unimplemented();
    }
    break;
  case 0x14: // w
    switch (mask_fun) {
    case 0x20:
      cvtsw(instr);
      break;
    case 0x21:
      cvtdw(instr);
      break;
    default:
      unimplemented();
    }
    break;
  case 0x15: // l
    switch (mask_fun) {
    case 0x20:
      cvtsl(instr);
      break;
    case 0x21:
      cvtdl(instr);
      break;
    default:
      unimplemented();
    }
    break;
  default:
    Util::panic("Unimplemented COP1 instruction {} {}", mask_sub >> 3, mask_sub & 7);
  }
}
} // namespace n64
