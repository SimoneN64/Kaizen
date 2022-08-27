#pragma once
#include <debugger_common.hpp>

inline std::string cop1decode(u32 instr) {
  u8 mask_sub = (instr >> 21) & 0x1F;
  u8 mask_fun = instr & 0x3F;
  u8 mask_branch = (instr >> 16) & 0x1F;
  switch(mask_sub) {
    // 000r_rccc
    case 0x00: return fmt::format("mfc1 {}, {}", gprStr[RT(instr)], fgrStr[FS(instr)]);
    case 0x01: return fmt::format("dmfc1 {}, {}", gprStr[RT(instr)], fgrStr[FS(instr)]);
    case 0x02: return fmt::format("cfc1 {}, {}", gprStr[RT(instr)], fgrStr[FS(instr)]);
    case 0x04: return fmt::format("mtc1 {}, {}", gprStr[RT(instr)], fgrStr[FS(instr)]);
    case 0x05: return fmt::format("dmtc1 {}, {}", gprStr[RT(instr)], fgrStr[FS(instr)]);
    case 0x06: return fmt::format("ctc1 {}, {}", gprStr[RT(instr)], fgrStr[FS(instr)]);
    case 0x08:
      switch(mask_branch) {
        case 0: return fmt::format("bcf {:04X}", instr & 0xffff);
        case 1: return fmt::format("bct {:04X}", instr & 0xffff);
        case 2: return fmt::format("bcfl {:04X}", instr & 0xffff);
        case 3: return fmt::format("bctl {:04X}", instr & 0xffff);
        default: return fmt::format("INVALID ({:08X})", instr);
      } break;
    case 0x10: // s
      switch(mask_fun) {
        case 0x00: return fmt::format("add.s {}, {}, {}", fgrStr[FD(instr)], fgrStr[FS(instr)], fgrStr[FT(instr)]);
        case 0x01: return fmt::format("sub.s {}, {}, {}", fgrStr[FD(instr)], fgrStr[FS(instr)], fgrStr[FT(instr)]);
        case 0x02: return fmt::format("mul.s {}, {}, {}", fgrStr[FD(instr)], fgrStr[FS(instr)], fgrStr[FT(instr)]);
        case 0x03: return fmt::format("div.s {}, {}, {}", fgrStr[FD(instr)], fgrStr[FS(instr)], fgrStr[FT(instr)]);
        case 0x04: return fmt::format("sqrt.s {}, {}", fgrStr[FD(instr)], fgrStr[FS(instr)]);
        case 0x05: return fmt::format("abs.s {}, {}", fgrStr[FD(instr)], fgrStr[FS(instr)]);
        case 0x06: return fmt::format("mov.s {}, {}", fgrStr[FD(instr)], fgrStr[FS(instr)]);
        case 0x07: return fmt::format("neg.s {}, {}", fgrStr[FD(instr)], fgrStr[FS(instr)]);
        case 0x08: return fmt::format("round.l.s {}, {}", fgrStr[FD(instr)], fgrStr[FS(instr)]);
        case 0x09: return fmt::format("trunc.l.s {}, {}", fgrStr[FD(instr)], fgrStr[FS(instr)]);
        case 0x0A: return fmt::format("ceil.l.s {}, {}", fgrStr[FD(instr)], fgrStr[FS(instr)]);
        case 0x0B: return fmt::format("floor.l.s {}, {}", fgrStr[FD(instr)], fgrStr[FS(instr)]);
        case 0x0C: return fmt::format("round.w.s {}, {}", fgrStr[FD(instr)], fgrStr[FS(instr)]);
        case 0x0D: return fmt::format("trunc.w.s {}, {}", fgrStr[FD(instr)], fgrStr[FS(instr)]);
        case 0x0E: return fmt::format("ceil.w.s {}, {}", fgrStr[FD(instr)], fgrStr[FS(instr)]);
        case 0x0F: return fmt::format("floor.w.s {}, {}", fgrStr[FD(instr)], fgrStr[FS(instr)]);
        case 0x21: return fmt::format("cvt.d.s {}, {}", fgrStr[FD(instr)], fgrStr[FS(instr)]);
        case 0x24: return fmt::format("cvt.w.s {}, {}", fgrStr[FD(instr)], fgrStr[FS(instr)]);
        case 0x25: return fmt::format("cvt.l.s {}, {}", fgrStr[FD(instr)], fgrStr[FS(instr)]);
        case 0x30: return fmt::format("c.f.s {}, {}", fgrStr[FS(instr)], fgrStr[FT(instr)]);
        case 0x31: return fmt::format("c.un.s {}, {}", fgrStr[FS(instr)], fgrStr[FT(instr)]);
        case 0x32: return fmt::format("c.eq.s {}, {}", fgrStr[FS(instr)], fgrStr[FT(instr)]);
        case 0x33: return fmt::format("c.ueq.s {}, {}", fgrStr[FS(instr)], fgrStr[FT(instr)]);
        case 0x34: return fmt::format("c.olt.s {}, {}", fgrStr[FS(instr)], fgrStr[FT(instr)]);
        case 0x35: return fmt::format("c.ult.s {}, {}", fgrStr[FS(instr)], fgrStr[FT(instr)]);
        case 0x36: return fmt::format("c.ole.s {}, {}", fgrStr[FS(instr)], fgrStr[FT(instr)]);
        case 0x37: return fmt::format("c.ule.s {}, {}", fgrStr[FS(instr)], fgrStr[FT(instr)]);
        case 0x38: return fmt::format("c.sf.s {}, {}", fgrStr[FS(instr)], fgrStr[FT(instr)]);
        case 0x39: return fmt::format("c.ngle.s {}, {}", fgrStr[FS(instr)], fgrStr[FT(instr)]);
        case 0x3A: return fmt::format("c.seq.s {}, {}", fgrStr[FS(instr)], fgrStr[FT(instr)]);
        case 0x3B: return fmt::format("c.ngl.s {}, {}", fgrStr[FS(instr)], fgrStr[FT(instr)]);
        case 0x3C: return fmt::format("c.lt.s {}, {}", fgrStr[FS(instr)], fgrStr[FT(instr)]);
        case 0x3D: return fmt::format("c.nge.s {}, {}", fgrStr[FS(instr)], fgrStr[FT(instr)]);
        case 0x3E: return fmt::format("c.le.s {}, {}", fgrStr[FS(instr)], fgrStr[FT(instr)]);
        case 0x3F: return fmt::format("c.ngt.s {}, {}", fgrStr[FS(instr)], fgrStr[FT(instr)]);
        default: return fmt::format("INVALID ({:08X})", instr);
      } break;
    case 0x11: // d
      switch(mask_fun) {
        case 0x00: return fmt::format("add.d {}, {}, {}", fgrStr[FD(instr)], fgrStr[FS(instr)], fgrStr[FT(instr)]);
        case 0x01: return fmt::format("sub.d {}, {}, {}", fgrStr[FD(instr)], fgrStr[FS(instr)], fgrStr[FT(instr)]);
        case 0x02: return fmt::format("mul.d {}, {}, {}", fgrStr[FD(instr)], fgrStr[FS(instr)], fgrStr[FT(instr)]);
        case 0x03: return fmt::format("div.d {}, {}, {}", fgrStr[FD(instr)], fgrStr[FS(instr)], fgrStr[FT(instr)]);
        case 0x04: return fmt::format("sqrt.d {}, {}", fgrStr[FD(instr)], fgrStr[FS(instr)]);
        case 0x05: return fmt::format("abs.d {}, {}", fgrStr[FD(instr)], fgrStr[FS(instr)]);
        case 0x06: return fmt::format("mov.d {}, {}", fgrStr[FD(instr)], fgrStr[FS(instr)]);
        case 0x07: return fmt::format("neg.d {}, {}", fgrStr[FD(instr)], fgrStr[FS(instr)]);
        case 0x08: return fmt::format("round.l.d {}, {}", fgrStr[FD(instr)], fgrStr[FS(instr)]);
        case 0x09: return fmt::format("trunc.l.d {}, {}", fgrStr[FD(instr)], fgrStr[FS(instr)]);
        case 0x0A: return fmt::format("ceil.l.d {}, {}", fgrStr[FD(instr)], fgrStr[FS(instr)]);
        case 0x0B: return fmt::format("floor.l.d {}, {}", fgrStr[FD(instr)], fgrStr[FS(instr)]);
        case 0x0C: return fmt::format("round.w.d {}, {}", fgrStr[FD(instr)], fgrStr[FS(instr)]);
        case 0x0D: return fmt::format("trunc.w.d {}, {}", fgrStr[FD(instr)], fgrStr[FS(instr)]);
        case 0x0E: return fmt::format("ceil.w.d {}, {}", fgrStr[FD(instr)], fgrStr[FS(instr)]);
        case 0x0F: return fmt::format("floor.w.d {}, {}", fgrStr[FD(instr)], fgrStr[FS(instr)]);
        case 0x20: return fmt::format("cvt.s.d {}, {}", fgrStr[FD(instr)], fgrStr[FS(instr)]);
        case 0x24: return fmt::format("cvt.w.d {}, {}", fgrStr[FD(instr)], fgrStr[FS(instr)]);
        case 0x25: return fmt::format("cvt.l.d {}, {}", fgrStr[FD(instr)], fgrStr[FS(instr)]);
        case 0x30: return fmt::format("c.f.d {}, {}", fgrStr[FS(instr)], fgrStr[FT(instr)]);
        case 0x31: return fmt::format("c.un.d {}, {}", fgrStr[FS(instr)], fgrStr[FT(instr)]);
        case 0x32: return fmt::format("c.eq.d {}, {}", fgrStr[FS(instr)], fgrStr[FT(instr)]);
        case 0x33: return fmt::format("c.ueq.d {}, {}", fgrStr[FS(instr)], fgrStr[FT(instr)]);
        case 0x34: return fmt::format("c.olt.d {}, {}", fgrStr[FS(instr)], fgrStr[FT(instr)]);
        case 0x35: return fmt::format("c.ult.d {}, {}", fgrStr[FS(instr)], fgrStr[FT(instr)]);
        case 0x36: return fmt::format("c.ole.d {}, {}", fgrStr[FS(instr)], fgrStr[FT(instr)]);
        case 0x37: return fmt::format("c.ule.d {}, {}", fgrStr[FS(instr)], fgrStr[FT(instr)]);
        case 0x38: return fmt::format("c.sf.d {}, {}", fgrStr[FS(instr)], fgrStr[FT(instr)]);
        case 0x39: return fmt::format("c.ngle.d {}, {}", fgrStr[FS(instr)], fgrStr[FT(instr)]);
        case 0x3A: return fmt::format("c.seq.d {}, {}", fgrStr[FS(instr)], fgrStr[FT(instr)]);
        case 0x3B: return fmt::format("c.ngl.d {}, {}", fgrStr[FS(instr)], fgrStr[FT(instr)]);
        case 0x3C: return fmt::format("c.lt.d {}, {}", fgrStr[FS(instr)], fgrStr[FT(instr)]);
        case 0x3D: return fmt::format("c.nge.d {}, {}", fgrStr[FS(instr)], fgrStr[FT(instr)]);
        case 0x3E: return fmt::format("c.le.d {}, {}", fgrStr[FS(instr)], fgrStr[FT(instr)]);
        case 0x3F: return fmt::format("c.ngt.d {}, {}", fgrStr[FS(instr)], fgrStr[FT(instr)]);
        default: return fmt::format("INVALID ({:08X})", instr);
      } break;
    case 0x14: // w
      switch(mask_fun) {
        case 0x01: return fmt::format("sub.w {}, {}, {}", fgrStr[FD(instr)], fgrStr[FS(instr)], fgrStr[FT(instr)]);
        case 0x02: return fmt::format("mul.w {}, {}, {}", fgrStr[FD(instr)], fgrStr[FS(instr)], fgrStr[FT(instr)]);
        case 0x05: return fmt::format("abs.w {}, {}", fgrStr[FD(instr)], fgrStr[FS(instr)]);
        case 0x06: return fmt::format("mov.w {}, {}", fgrStr[FD(instr)], fgrStr[FS(instr)]);
        case 0x20: return fmt::format("cvt.s.w {}, {}", fgrStr[FD(instr)], fgrStr[FS(instr)]);
        case 0x21: return fmt::format("cvt.d.w {}, {}", fgrStr[FD(instr)], fgrStr[FS(instr)]);
        default: return fmt::format("INVALID ({:08X})", instr);
      } break;
    case 0x15: // l
      switch(mask_fun) {
        case 0x01: return fmt::format("sub.l {}, {}, {}", fgrStr[FD(instr)], fgrStr[FS(instr)], fgrStr[FT(instr)]);
        case 0x02: return fmt::format("mul.l {}, {}, {}", fgrStr[FD(instr)], fgrStr[FS(instr)], fgrStr[FT(instr)]);
        case 0x05: return fmt::format("abs.l {}, {}", fgrStr[FD(instr)], fgrStr[FS(instr)]);
        case 0x06: return fmt::format("mov.l {}, {}", fgrStr[FD(instr)], fgrStr[FS(instr)]);
        case 0x20: return fmt::format("cvt.s.l {}, {}", fgrStr[FD(instr)], fgrStr[FS(instr)]);
        case 0x21: return fmt::format("cvt.d.l {}, {}", fgrStr[FD(instr)], fgrStr[FS(instr)]);
        default: return fmt::format("INVALID ({:08X})", instr);
      } break;
    default: return fmt::format("INVALID ({:08X})", instr);
  }
}