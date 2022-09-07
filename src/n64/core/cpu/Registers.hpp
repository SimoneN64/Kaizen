#pragma once
#include <Cop0.hpp>
#include <Cop1.hpp>

namespace n64 {
struct Registers {
  Registers();
  void Reset();
  void SetPC(s64);
  s64 gpr[32];
  Cop0 cop0;
  Cop1 cop1;
  s64 oldPC, pc, nextPC;
  s64 hi, lo;
  bool prevDelaySlot, delaySlot;
};

constexpr char* gprStr[32] = {
  "zero", // 0
  "at",   // 1
  "v0",   // 2
  "v1",   // 3
  "a0",   // 4
  "a1",   // 5
  "a2",   // 6
  "a3",   // 7
  "t0",   // 8
  "t1",   // 9
  "t2",   // 10
  "t3",   // 11
  "t4",   // 12
  "t5",   // 13
  "t6",   // 14
  "t7",   // 15
  "s0",   // 16
  "s1",   // 17
  "s2",   // 18
  "s3",   // 19
  "s4",   // 20
  "s5",   // 21
  "s6",   // 22
  "s7",   // 23
  "t8",   // 24
  "t9",   // 25
  "k0",   // 26
  "k1",   // 27
  "gp",   // 28
  "sp",   // 29
  "s8",   // 30
  "ra"    // 31
};

}
