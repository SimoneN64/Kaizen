#include <IR.hpp>
#include <fmt/format.h>

namespace n64 {
template <> struct fmt::formatter<Entry> : formatter<string_view> {
  auto format(Entry e, format_context& ctx) const {
    std::string op = "Unknown";
    switch (e.op) {
      case Entry::MOV:
        op = "MOV";
        break;
      case Entry::ADD:
        op = "ADD";
        break;
      case Entry::SUB:
        op = "SUB";
        break;
      case Entry::UMUL:
        op = "UMUL";
        break;
      case Entry::SMUL:
        op = "SMUL";
        break;
      case Entry::DIV:
        op = "DIV";
        break;
      case Entry::AND:
        op = "AND";
        break;
      case Entry::NOR:
        op = "NOR";
        break;
      case Entry::XOR:
        op = "XOR";
        break;
      case Entry::OR:
        op = "OR";
        break;
      case Entry::SRL:
        op = "SRL";
        break;
      case Entry::SLL:
        op = "SLL";
        break;
      case Entry::SRA:
        op = "SRA";
        break;
      case Entry::LOADS8:
        op = "LOADS8";
        break;
      case Entry::LOADS8_SHIFT:
        op = "LOADS8_SHIFT";
        break;
      case Entry::STORE8:
        op = "STORE8";
        break;
      case Entry::STORE8_SHIFT:
        op = "STORE8_SHIFT";
        break;
      case Entry::LOADS16:
        op = "LOADS16";
        break;
      case Entry::LOADS16_SHIFT:
        op = "LOADS16_SHIFT";
        break;
      case Entry::STORE16:
        op = "STORE16";
        break;
      case Entry::STORE16_SHIFT:
        op = "STORE16_SHIFT";
        break;
      case Entry::LOADS32:
        op = "LOADS32";
        break;
      case Entry::LOADS32_SHIFT:
        op = "LOADS32_SHIFT";
        break;
      case Entry::STORE32:
        op = "STORE32";
        break;
      case Entry::STORE32_SHIFT:
        op = "STORE32_SHIFT";
        break;
      case Entry::LOADS64:
        op = "LOADS64";
        break;
      case Entry::LOADS64_SHIFT:
        op = "LOADS64_SHIFT";
        break;
      case Entry::STORE64:
        op = "STORE64";
        break;
      case Entry::STORE64_SHIFT:
        op = "STORE64_SHIFT";
        break;
      case Entry::LOADU8:
        op = "LOADU8";
        break;
      case Entry::LOADU8_SHIFT:
        op = "LOADU8_SHIFT";
        break;
      case Entry::LOADU16:
        op = "LOADU16";
        break;
      case Entry::LOADU16_SHIFT:
        op = "LOADU16_SHIFT";
        break;
      case Entry::LOADU32:
        op = "LOADU32";
        break;
      case Entry::LOADU32_SHIFT:
        op = "LOADU32_SHIFT";
        break;
      case Entry::LOADU64:
        op = "LOADU64";
        break;
      case Entry::LOADU64_SHIFT:
        op = "LOADU64_SHIFT";
        break;
      case Entry::BRANCH:
        op = "BRANCH";
        break;
      case Entry::JUMP:
        op = "JUMP";
        break;
    }

    bool put_comma = false;
    op += " ";
    if (e.dst.isReg()) {
      if (e.dst.index_or_imm.has_value()) {
        if (e.dst.index_or_imm.value() == 0) {
          op = "NOP";
          return formatter<string_view>::format(op, ctx);
        } else {
          std::string dst = fmt::format("R{}", e.dst.index_or_imm.value());
          op += dst;
          put_comma = true;
        }
      }
    } else {
      if(e.dst.type != Entry::Operand::NONE) {
        std::string dst = fmt::format("0x{:0X}", e.dst.index_or_imm.value());
        op += dst;
        put_comma = true;
      }
    }

    if (e.op1.isReg()) {
      if (e.op1.index_or_imm.has_value()) {
        std::string op1 = fmt::format("R{}", e.op1.index_or_imm.value());
        if(put_comma) {
          op += ", ";
        } else {
          put_comma = true;
        }
        op += op1;
      }
    } else {
      if (e.op1.index_or_imm.has_value()) {
        std::string op1 = fmt::format("0x{:0X}", e.op1.index_or_imm.value());
        if(put_comma) {
          op += ", ";
        } else {
          put_comma = true;
        }
        op += op1;
      }
    }

    if (e.op2.isReg()) {
      if (e.op2.index_or_imm.has_value()) {
        std::string op2 = fmt::format("R{}", e.op2.index_or_imm.value());
        if(put_comma) {
          op += ", ";
        } else {
          put_comma = true;
        }
        op += op2;
      }
    } else {
      if (e.op2.index_or_imm.has_value()) {
        std::string op2 = fmt::format("0x{:0X}", e.op2.index_or_imm.value());
        if(put_comma) {
          op += ", ";
        } else {
          put_comma = true;
        }
        op += op2;
      }
    }

    op += '\n';
    return formatter<string_view>::format(op, ctx);
  }
};

Entry::Entry(Opcode op, Operand dst, Operand op1, Operand op2)
	: op(op), dst(dst), op1(op1), op2(op2) {}

Entry::Entry(Opcode op, Operand op1, Operand op2)
	: op(op), op1(op1), op2(op2) {}

Entry::Entry(Opcode op, Operand bDest, Operand op1, std::optional<BranchCond> bc, Operand op2)
	: op(op), bDest(bDest), op1(op1), branchCond(bc), op2(op2) {}

Entry::Entry(Opcode op, Operand bDest)
: op(op), bDest(bDest) {}

Entry::Entry(Opcode op, Operand dst, Operand op1, Operand op2, Shift s)
	: op(op), dst(dst), op1(op1), op2(op2), shift(s) {}

void IR::push(const Entry& e) {
	code.push_back(e);
}

void IR::optimize() {
  std::vector<Entry> optimized{};

  for(const auto& i : code) {
    bool isOp1Reg = i.op1.isReg();
    bool isOp2Reg = i.op2.isReg();
    bool isDstReg = i.dst.isReg();

    if(isDstReg) {
      if(isOp1Reg) {
        if(i.op1.index_or_imm == i.dst.index_or_imm
          && i.zeroRendersItUseless()) continue;
      }

      if(isOp2Reg) {
        if(i.op2.index_or_imm == i.dst.index_or_imm
           && i.zeroRendersItUseless()) continue;
      }
    }

    optimized.push_back(i);
  }

  if(optimized.size() == code.size()) {
    return;
  }

  code = optimized;
  optimize();
}

void IR::print() {
  for(auto e : code) {
    fmt::print("{}", e);
  }
}

auto IR::begin() {
	return code.begin();
}

auto IR::end() {
	return code.end();
}
}