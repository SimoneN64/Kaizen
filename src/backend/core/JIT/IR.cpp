#include <IR.hpp>

namespace n64 {
Entry::Entry(Opcode op, Operand dst, Operand op1, Operand op2)
	: op(op), dst(dst), op1(op1), op2(op2) {}

Entry::Entry(Opcode op, Operand op1, Operand op2)
	: op(op), op1(op1), op2(op2) {}

Entry::Entry(Opcode op, Operand op1, std::optional<BranchCond> bc, Operand op2)
	: op(op), op1(op1), branchCond(bc), op2(op2) {}

Entry::Entry(Opcode op, Operand dst, Operand op1, Operand op2, Shift s)
	: op(op), dst(dst), op1(op1), op2(op2), shift(s) {}

void IR::push(const Entry& e) {
	code.push_back(e);
}

auto IR::begin() {
	return code.begin();
}

auto IR::end() {
	return code.end();
}
}