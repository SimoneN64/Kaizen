#pragma once
#include <common.hpp>

static const std::string gprStr[32] = {
  "zero", "at", "v0", "v1",
  "a0", "a1", "a2", "a3",
  "t0", "t1", "t2", "t3",
  "t4", "t5", "t6", "t7",
  "s0", "s1", "s2", "s3",
  "s4", "s5", "s6", "s7",
  "t8", "t9", "k0", "k1",
  "gp", "sp", "s8", "ra"
};

static const std::string cop0Str[32] = {
  "Index", "Random", "EntryLo0", "EntryLo1",
  "Context", "PageMask", "Wired", "r7",
  "BadVAddr", "Count", "EntryHi", "Compare",
  "Status", "Cause", "EPC", "PRId",
  "Config", "LLAddr", "WatchLo", "WatchHi",
  "XContext", "r21", "r22", "r23",
  "r24", "r25", "Parity Error", "Cache Error",
  "TagLo", "TagHi", "ErrorEPC", "r31"
};

static const std::string fgrStr[32] = {
  "fgr0", "fgr1", "fgr2", "fgr3",
  "fgr4", "fgr5", "fgr6", "fgr7",
  "fgr8", "fgr9", "fgr10", "fgr11",
  "fgr12", "fgr13", "fgr14", "fgr15",
  "fgr16", "fgr17", "fgr18", "fgr19",
  "fgr20", "fgr21", "fgr22", "fgr23",
  "fgr24", "fgr25", "fgr26", "fgr27",
  "fgr28", "fgr29", "fgr30", "fgr31"
};

static const std::string vprStr[32] = {
  "vpr0", "vpr1", "vpr2", "vpr3",
  "vpr4", "vpr5", "vpr6", "vpr7",
  "vpr8", "vpr9", "vpr10", "vpr11",
  "vpr12", "vpr13", "vpr14", "vpr15",
  "vpr16", "vpr17", "vpr18", "vpr19",
  "vpr20", "vpr21", "vpr22", "vpr23",
  "vpr24", "vpr25", "vpr26", "vpr27",
  "vpr28", "vpr29", "vpr30", "vpr31"
};

static const std::string vprByteStr[16] = {
  "e0_8", "e1_8", "e2_8", "e3_8",
  "e4_8", "e5_8", "e6_8", "e7_8",
  "e8_8", "e9_8", "e10_8", "e11_8",
  "e12_8", "e13_8", "e14_8", "e15_8"
};

static const std::string vprElementStr[8] = {
  "e0_16", "e2_16", "e4_16", "e6_16",
  "e8_16", "e10_16", "e12_16", "e14_16"
};

static const std::string vprWordStr[4] = {
  "e0_32", "e4_32", "e8_32", "e12_32"
};

static const std::string rspControlStr[4] = {
  "vco", "vcc", "vce", "vce"
};