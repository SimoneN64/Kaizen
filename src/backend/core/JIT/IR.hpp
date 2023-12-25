#pragma once
#include <common.hpp>
#include <vector>
#include <optional>

namespace n64 {
struct Entry {
	enum : u16 {
		LINK = 0x100,
		LIKELY = 0x200,
		REGISTER = 0x400,
		SET_LLBIT = 0x800
	};

	enum Shift {
		LEFT, RIGHT
	};

	enum Opcode : u16 {
		MOV, ADD, SUB, UMUL, SMUL, DIV, AND, NOR, XOR, OR, SRL, SLL, SRA,
		LOADS8,  LOADS8_SHIFT,  STORE8,  STORE8_SHIFT,
		LOADS16, LOADS16_SHIFT, STORE16, STORE16_SHIFT,
		LOADS32, LOADS32_SHIFT, STORE32, STORE32_SHIFT, 
		LOADS64, LOADS64_SHIFT, STORE64, STORE64_SHIFT,
		LOADU8,  LOADU8_SHIFT,
		LOADU16, LOADU16_SHIFT,
		LOADU32, LOADU32_SHIFT,
		LOADU64, LOADU64_SHIFT,
		BRANCH, JUMP,
	} op;

	struct Operand {
		enum Type {
			NONE, REG_F64, REG_F32, IMM_F64, IMM_F32,
			REG_S64, REG_S32, REG_U64, REG_U32, REG_U5, IMM_S16,
			IMM_S32, IMM_S64, IMM_U16, IMM_U32, IMM_U64, IMM_U5,
			MEM_U8, MEM_U16, MEM_U32, MEM_U64, PC64, NEXTPC64,
      LO, HI
		} type = NONE;

    bool isReg() {
      return type == REG_S64 || type == REG_F32 || type == REG_F64 || type == REG_S32
              || type == REG_U64 || type == REG_U32 || type == REG_U5;
    }

		std::optional<u64> index_or_imm = std::nullopt;

    Operand() = default;
		Operand(Type t, std::optional <u64> imm = std::nullopt)
			: type(t), index_or_imm(imm) {}
	} dst, op1, op2;

  [[nodiscard]] const Operand& GetDst() const { return dst; }

	enum BranchCond {
		AL, EQ, NE, LT, GT, LE, GE
	};

	std::optional<BranchCond> branchCond = std::nullopt;
	std::optional<Shift> shift = std::nullopt;
  Operand bDest = Operand::NONE;

	Entry(Opcode op, Operand dst, Operand op1, Operand op2);
	Entry(Opcode op, Operand op1, Operand op2);
	Entry(Opcode op, Operand bDest, Operand op1, std::optional<BranchCond> bc, Operand op2);
  Entry(Opcode op, Operand bDest);
	Entry(Opcode op, Operand dst, Operand op1, Operand op2, Shift s);
};

struct IR {
	void push(const Entry&);
	auto begin();
	auto end();
  void optimize();
private:
	std::vector<Entry> code{};
};
}