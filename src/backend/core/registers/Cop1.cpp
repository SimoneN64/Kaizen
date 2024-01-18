#include <core/registers/Cop1.hpp>
#include <core/registers/Registers.hpp>
#include <core/Interpreter.hpp>
#include <log.hpp>

namespace n64 {
Cop1::Cop1() {
  Reset();
}

void Cop1::Reset() {
  fcr0 = 0xa00;
  fcr31.write(0x01000800);
  memset(fgr, 0, 32 * sizeof(FloatingPointReg));
}

template <class T>
void Cop1::decode(T& cpu, u32 instr) {
  if constexpr (std::is_same_v<decltype(cpu), Interpreter&>) {
    decodeInterp(cpu, instr);
  } else {
    Util::panic("What the fuck did you just give me?!");
  }
}

template void Cop1::decode<Interpreter>(Interpreter&, u32);
template void Cop1::decode<JIT>(JIT&, u32);

void Cop1::decodeInterp(Interpreter &cpu, u32 instr) {
  Registers &regs = cpu.regs;

  u8 mask_sub = (instr >> 21) & 0x1F;
  u8 mask_fun = instr & 0x3F;
  u8 mask_branch = (instr >> 16) & 0x1F;
  switch(mask_sub) {
    // 000r_rccc
    case 0x00: mfc1(regs, instr); break;
    case 0x01: dmfc1(regs, instr); break;
    case 0x02: cfc1(regs, instr); break;
    case 0x03: unimplemented(regs); break;
    case 0x04: mtc1(regs, instr); break;
    case 0x05: dmtc1(regs, instr); break;
    case 0x06: ctc1(regs, instr); break;
    case 0x07: unimplemented(regs); break;
    case 0x08:
      switch(mask_branch) {
        case 0: CheckFPUUsable(); cpu.b(instr, !regs.cop1.fcr31.compare); break;
        case 1: CheckFPUUsable(); cpu.b(instr, regs.cop1.fcr31.compare); break;
        case 2: CheckFPUUsable(); cpu.bl(instr, !regs.cop1.fcr31.compare); break;
        case 3: CheckFPUUsable(); cpu.bl(instr, regs.cop1.fcr31.compare); break;
        default: Util::panic("Undefined BC COP1 {:02X}", mask_branch);
      }
      break;
    case 0x10: // s
      switch(mask_fun) {
        case 0x00: adds(regs, instr); break;
        case 0x01: subs(regs, instr); break;
        case 0x02: muls(regs, instr); break;
        case 0x03: divs(regs, instr); break;
        case 0x04: sqrts(regs, instr); break;
        case 0x05: abss(regs, instr); break;
        case 0x06: movs(regs, instr); break;
        case 0x07: negs(regs, instr); break;
        case 0x08: roundls(regs, instr); break;
        case 0x09: truncls(regs, instr); break;
        case 0x0A: ceills(regs, instr); break;
        case 0x0B: floorls(regs, instr); break;
        case 0x0C: roundws(regs, instr); break;
        case 0x0D: truncws(regs, instr); break;
        case 0x0E: ceilws(regs, instr); break;
        case 0x0F: floorws(regs, instr); break;
        case 0x21: cvtds(regs, instr); break;
        case 0x24: cvtws(regs, instr); break;
        case 0x25: cvtls(regs, instr); break;
        case 0x30: cf<float>(regs, instr); break;
        case 0x31: cun<float>(regs, instr); break;
        case 0x32: ceq<float>(regs, instr); break;
        case 0x33: cueq<float>(regs, instr); break;
        case 0x34: colt<float>(regs, instr); break;
        case 0x35: cult<float>(regs, instr); break;
        case 0x36: cole<float>(regs, instr); break;
        case 0x37: cule<float>(regs, instr); break;
        case 0x38: csf<float>(regs, instr); break;
        case 0x39: cngle<float>(regs, instr); break;
        case 0x3A: cseq<float>(regs, instr); break;
        case 0x3B: cngl<float>(regs, instr); break;
        case 0x3C: clt<float>(regs, instr); break;
        case 0x3D: cnge<float>(regs, instr); break;
        case 0x3E: cle<float>(regs, instr); break;
        case 0x3F: cngt<float>(regs, instr); break;
        default: unimplemented(regs);
      }
      break;
    case 0x11: // d
      switch(mask_fun) {
        case 0x00: addd(regs, instr); break;
        case 0x01: subd(regs, instr); break;
        case 0x02: muld(regs, instr); break;
        case 0x03: divd(regs, instr); break;
        case 0x04: sqrtd(regs, instr); break;
        case 0x05: absd(regs, instr); break;
        case 0x06: movd(regs, instr); break;
        case 0x07: negd(regs, instr); break;
        case 0x08: roundld(regs, instr); break;
        case 0x09: truncld(regs, instr); break;
        case 0x0A: ceilld(regs, instr); break;
        case 0x0B: floorld(regs, instr); break;
        case 0x0C: roundwd(regs, instr); break;
        case 0x0D: truncwd(regs, instr); break;
        case 0x0E: ceilwd(regs, instr); break;
        case 0x0F: floorwd(regs, instr); break;
        case 0x20: cvtsd(regs, instr); break;
        case 0x24: cvtwd(regs, instr); break;
        case 0x25: cvtld(regs, instr); break;
        case 0x30: cf<double>(regs, instr); break;
        case 0x31: cun<double>(regs, instr); break;
        case 0x32: ceq<double>(regs, instr); break;
        case 0x33: cueq<double>(regs, instr); break;
        case 0x34: colt<double>(regs, instr); break;
        case 0x35: cult<double>(regs, instr); break;
        case 0x36: cole<double>(regs, instr); break;
        case 0x37: cule<double>(regs, instr); break;
        case 0x38: csf<double>(regs, instr); break;
        case 0x39: cngle<double>(regs, instr); break;
        case 0x3A: cseq<double>(regs, instr); break;
        case 0x3B: cngl<double>(regs, instr); break;
        case 0x3C: clt<double>(regs, instr); break;
        case 0x3D: cnge<double>(regs, instr); break;
        case 0x3E: cle<double>(regs, instr); break;
        case 0x3F: cngt<double>(regs, instr); break;
        default: unimplemented(regs);
      }
      break;
    case 0x14: // w
      switch(mask_fun) {
        case 0x20: cvtsw(regs, instr); break;
        case 0x21: cvtdw(regs, instr); break;
        default: unimplemented(regs);
      }
      break;
    case 0x15: // l
      switch(mask_fun) {
        case 0x20: cvtsl(regs, instr); break;
        case 0x21: cvtdl(regs, instr); break;
        default: unimplemented(regs);
      }
      break;
    default: Util::panic("Unimplemented COP1 instruction {} {}", mask_sub >> 3, mask_sub & 7);
  }
}
}