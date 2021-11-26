#pragma once
#include <mem.h>
#include <registers.h>

typedef struct {
  bool halt, skip, ime, ei;
  u8 cycles, opcode;
  registers_t registers;
} cpu_t;

void init_cpu(cpu_t* cpu, bool skip);
void destroy_cpu(cpu_t *cpu);
void handle_interrupts(cpu_t* cpu);

INLINE void reset(cpu_t* cpu) {
  bool skip = cpu->skip;
  destroy_cpu(cpu);
  init_cpu(cpu, skip);
}

u8 step_cpu(cpu_t* cpu);