#pragma once
#include <common.hpp>

const std::string gprStr[32] = {
  "zero", "at", "v0", "v1",
  "a0", "a1", "a2", "a3",
  "t0", "t1", "t2", "t3",
  "t4", "t5", "t6", "t7",
  "s0", "s1", "s2", "s3",
  "s4", "s5", "s6", "s7",
  "t8", "t9", "k0", "k1",
  "gp", "sp", "s8", "ra"
};

const std::string cop0Str[32] = {
  "Index", "Random", "EntryLo0", "EntryLo1",
  "Context", "PageMask", "Wired", "r7",
  "BadVAddr", "Count", "EntryHi", "Compare",
  "Status", "Cause", "EPC", "PRId",
  "Config", "LLAddr", "WatchLo", "WatchHi",
  "XContext", "r21", "r22", "r23",
  "r24", "r25", "Parity Error", "Cache Error",
  "TagLo", "TagHi", "ErrorEPC", "r31"
};

const std::string fgrStr[32] = {
  "fgr0", "fgr1", "fgr2", "fgr3",
  "fgr4", "fgr5", "fgr6", "fgr7",
  "fgr8", "fgr9", "fgr10", "fgr11",
  "fgr12", "fgr13", "fgr14", "fgr15",
  "fgr16", "fgr17", "fgr18", "fgr19",
  "fgr20", "fgr21", "fgr22", "fgr23",
  "fgr24", "fgr25", "fgr26", "fgr27",
  "fgr28", "fgr29", "fgr30", "fgr31"
};

const std::string rspStr[] = {

};