#pragma once
#include <utils.h>
#include <mem.h>

typedef struct {
  REGS_UNION(a, f);
  REGS_UNION(b, c);
  REGS_UNION(d, e);
  REGS_UNION(h, l);

  u16 sp, pc;
} registers_t;

INLINE bool get_cond(registers_t* regs, u8 opcode) {
  if (opcode & 1) {
    return true;
  }
  u8 bits = (opcode >> 3) & 3;
  switch (bits) {
    case 0: return !regs->f.z;
    case 1: return regs->f.z;
    case 2: return !regs->f.c;
    case 3: return regs->f.c;
  }
}

INLINE void reg_push(registers_t* regs, mem_t* mem, u16 val) {
  regs->sp -= 2;
  write16(mem, regs->sp, val);
}

INLINE u16 reg_pop(registers_t* regs, mem_t* mem) {
  u16 val = read16(mem, regs->sp);
  regs->sp += 2;
  return val;
}

INLINE u8 read_reg8(registers_t* regs, mem_t* mem, u8 bits) {
  switch (bits) {
    case 0: return regs->b;
    case 1: return regs->c;
    case 2: return regs->d;
    case 3: return regs->e;
    case 4: return regs->h;
    case 5: return regs->l;
    case 6: return read8(mem, regs->hl);
    case 7: return regs->a;
  }
}

INLINE void write_reg8(registers_t* regs, mem_t* mem, u8 bits, u8 val) {
  switch (bits) {
    case 0: regs->b = val; break;
    case 1: regs->c = val; break;
    case 2: regs->d = val; break;
    case 3: regs->e = val; break;
    case 4: regs->h = val; break;
    case 5: regs->l = val; break;
    case 6: write8(mem, regs->hl, val); break;
    case 7: regs->a = val; break;
  }
}

INLINE u16 read_reg16(registers_t* regs, int group, u8 bits) {
  if (group == 1) {
    switch (bits) {
      case 0: return regs.bc;
      case 1: return regs.de;
      case 2: return regs.hl;
      case 3: return regs.sp;
    }
  } else if (group == 2) {
    switch (bits) {
      case 0: return regs.bc;
      case 1: return regs.de;
      case 2: case 3: return regs.hl;
    }
  } else if (group == 3) {
    switch (bits) {
      case 0: return regs.bc;
      case 1: return regs.de;
      case 2: return regs.hl;
      case 3: return regs.af;
    }
  }
}

INLINE void write_reg16(registers_t* regs, int group, u8 bits, u16 value) {
  if (group == 1) {
    switch (bits) {
    case 0: regs.bc = value; break;
    case 1: regs.de = value; break;
    case 2: regs.hl = value; break;
    case 3: regs.sp = value; break;
    }
  } else if (group == 2) {
    switch (bits) {
      case 0: regs.bc = value; break;
      case 1: regs.de = value; break;
      case 2: case 3: regs.hl = value; break;
    }
  } else if (group == 3) {
    switch (bits) {
      case 0: regs.bc = value; break;
      case 1: regs.de = value; break;
      case 2: regs.hl = value; break;
      case 3:
        regs->a = value >> 8;
        regs->f = value;
        break;
    }
  }
}