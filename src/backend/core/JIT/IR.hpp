#pragma once
#include <common.hpp>
#include <vector>

namespace n64 {
struct Entry {
	u16 type;
	struct Operand {
		enum {
			REG_S64, REG_S32, REG_U64, REG_U32, REG_U5, IMM_S16,
			IMM_S32, IMM_S64, IMM_U16, IMM_U32, IMM_U64, IMM_U5,
		} type;
		u8 index;
	} dst, op1, op2;
};

struct IR {
	void push(const Entry&);
	auto begin();
	auto end();
private:
	std::vector<Entry> code{};
};
}