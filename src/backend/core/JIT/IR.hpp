#pragma once
#include <common.hpp>
#include <vector>
#include <optional>

namespace n64 {
struct Entry {
	enum : u8 {
		LINK = 0x10,
		LIKELY = 0x20,
		REGISTER = 0x40
	};

	enum Opcode : u16 {
		MOV, ADD, SUB, MUL, DIV, AND, NOR, XOR, OR, SRL, SLL, SRA,
		LOAD, STORE, BRANCH, JUMP
	} op;

	struct Operand {
		enum Type {
			NONE, REG_F64, REG_F32, IMM_F64, IMM_F32,
			REG_S64, REG_S32, REG_U64, REG_U32, REG_U5, IMM_S16,
			IMM_S32, IMM_S64, IMM_U16, IMM_U32, IMM_U64, IMM_U5,
		} type;
		std::optional<u64> index_or_imm;

		Operand(Type t, std::optional<u64> i = std::nullopt)
			: type(t), index_or_imm(i) {}
	} dst = Operand::NONE, op1, op2;

	enum BranchCond {
		EQ, NE, LT, GT, LE, GE
	};

	std::optional<BranchCond> branchCond = std::nullopt;

	Entry(Opcode op, Operand dst, Operand op1, Operand op2) 
		: op(op), dst(dst), op1(op1), op2(op2) {}

	Entry(Opcode op, Operand op1, Operand op2)
		: op(op), op1(op1), op2(op2) {}

	Entry(Opcode op, Operand op1, std::optional<BranchCond> bc, Operand op2)
		: op(op), op1(op1), branchCond(bc), op2(op2) {}
};

struct IR {
	void push(const Entry&);
	auto begin();
	auto end();
private:
	std::vector<Entry> code{};
};
}