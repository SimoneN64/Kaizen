#include <IR.hpp>
#include <fmt/format.h>

namespace n64 {
template <> struct fmt::formatter<Entry> : formatter<string_view> {
  auto format(Entry e, format_context& ctx) const {
    std::string op = "Unknown";
    switch (e.op) {
      case Entry::MOV: op = "MOV"; break;
      case Entry::ADD: op = "ADD"; break;
      case Entry::SUB: op = "SUB"; break;
      case Entry::UMUL: op = "UMUL"; break;
      case Entry::SMUL: op = "SMUL"; break;
      case Entry::DIV: op = "DIV"; break;
      case Entry::AND: op = "AND"; break;
      case Entry::NOR: op = "NOR"; break;
      case Entry::XOR: op = "XOR"; break;
      case Entry::OR: op = "OR"; break;
      case Entry::SRL: op = "SRL"; break;
      case Entry::SLL: op = "SLL"; break;
      case Entry::SRA: op = "SRA"; break;
      case Entry::LOADS8: op = "LOADS8"; break;
      case Entry::LOADS8_SHIFT: op = "LOADS8_SHIFT"; break;
      case Entry::STORE8: op = "STORE8"; break;
      case Entry::STORE8_SHIFT: op = "STORE8_SHIFT"; break;
      case Entry::LOADS16: op = "LOADS16"; break;
      case Entry::LOADS16_SHIFT: op = "LOADS16_SHIFT"; break;
      case Entry::STORE16: op = "STORE16"; break;
      case Entry::STORE16_SHIFT: op = "STORE16_SHIFT"; break;
      case Entry::LOADS32: op = "LOADS32"; break;
      case Entry::LOADS32_SHIFT: op = "LOADS32_SHIFT"; break;
      case Entry::STORE32: op = "STORE32"; break;
      case Entry::STORE32_SHIFT: op = "STORE32_SHIFT"; break;
      case Entry::LOADS64: op = "LOADS64"; break;
      case Entry::LOADS64_SHIFT: op = "LOADS64_SHIFT"; break;
      case Entry::STORE64: op = "STORE64"; break;
      case Entry::STORE64_SHIFT: op = "STORE64_SHIFT"; break;
      case Entry::LOADU8: op = "LOADU8"; break;
      case Entry::LOADU8_SHIFT: op = "LOADU8_SHIFT"; break;
      case Entry::LOADU16: op = "LOADU16"; break;
      case Entry::LOADU16_SHIFT: op = "LOADU16_SHIFT"; break;
      case Entry::LOADU32: op = "LOADU32"; break;
      case Entry::LOADU32_SHIFT: op = "LOADU32_SHIFT"; break;
      case Entry::LOADU64: op = "LOADU64"; break;
      case Entry::LOADU64_SHIFT: op = "LOADU64_SHIFT"; break;
      case Entry::BRANCH: op = "BRANCH"; break;
      case Entry::JUMP: op = "JUMP"; break;
      case Entry::MTC0: op = "MTC0"; break;
      case Entry::MFC0: op = "MFC0"; break;
      case Entry::SLT: op = "SLT"; break;
    }

    bool put_comma = false;
    op += " ";
    if (e.dst.isReg()) {
      if (e.dst.index_or_imm.has_value()) {
        std::string dst = fmt::format("R{}", e.dst.index_or_imm.value());
        op += dst;
        put_comma = true;
      }
    } else if(e.dst.isImm()) {
      std::string dst = fmt::format("0x{:0X}", e.dst.index_or_imm.value());
      op += dst;
      put_comma = true;
    } else if(e.dst.type == Entry::Operand::PC64) {
      std::string dst = fmt::format("PC");
      op += dst;
      put_comma = true;
    } else {
      if(e.dst.type != Entry::Operand::NONE) {
        std::string dst = fmt::format("(0x{:0X})", e.dst.index_or_imm.value());
        op += dst;
        put_comma = true;
      }
    }

    if (e.bOffs.index_or_imm.has_value()) {
      std::string dst = fmt::format("0x{:0X}", e.bOffs.index_or_imm.value());
      op += dst;
      put_comma = true;
    }

    if (e.op1.isReg()) {
      if (e.op1.index_or_imm.has_value()) {
        std::string op1 = fmt::format("R{}", e.op1.index_or_imm.value());
        if(put_comma) {
          op += ", ";
        }
        op += op1;
        put_comma = true;
      }
    } else if(e.op1.isImm()) {
      if (e.op1.index_or_imm.has_value()) {
        std::string op1 = fmt::format("0x{:0X}", e.op1.index_or_imm.value());
        if(put_comma) {
          op += ", ";
        }
        op += op1;
        put_comma = true;
      }
    } else if (e.dst.type == Entry::Operand::PC64) {
      std::string dst = fmt::format("PC");
      if (put_comma) {
        op += ", ";
      }
      op += dst;
      put_comma = true;
    } else {
      if (e.op1.index_or_imm.has_value()) {
        std::string op1 = fmt::format("(0x{:0X})", e.op1.index_or_imm.value());
        if(put_comma) {
          op += ", ";
        }
        op += op1;
        put_comma = false;
      }
    }

    if (e.branchCond.has_value()) {
      put_comma = false;
      op += " ";
      switch (e.branchCond.value()) {
      case Entry::AL: op += "   "; break;
      case Entry::EQ: op += "== "; break;
      case Entry::NE: op += "!= "; break;
      case Entry::LT: op += "<  "; break;
      case Entry::GT: op += ">  "; break;
      case Entry::LE: op += "<= "; break;
      case Entry::GE: op += ">= "; break;
      }
    }

    if (e.op2.isReg()) {
      if (e.op2.index_or_imm.has_value()) {
        std::string op2 = fmt::format("R{}", e.op2.index_or_imm.value());
        if(put_comma) {
          op += ", ";
        }
        op += op2;
      }
    } else {
      if (e.op2.index_or_imm.has_value()) {
        std::string op2 = fmt::format("0x{:0X}", e.op2.index_or_imm.value());
        if(put_comma) {
          op += ", ";
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

Entry::Entry(Opcode op, Operand bOffs, Operand op1, std::optional<BranchCond> bc, Operand op2)
	: op(op), bOffs(bOffs), op1(op1), branchCond(bc), op2(op2) {}

Entry::Entry(Opcode op, Operand bOffs)
: op(op), bOffs(bOffs) {}

Entry::Entry(Opcode op, Operand dst, Operand op1, Operand op2, Shift s)
	: op(op), dst(dst), op1(op1), op2(op2), shift(s) {}

void IR::push(const Entry& e) {
	code.push_back(e);
}

void IR::dead_code_elimination(std::vector<Entry>& code_) {
  for(const auto& i : code) {
    bool isOp1Reg = i.op1.isReg();
    bool isOp2Reg = i.op2.isReg();
    bool isDstReg = i.dst.isReg();

    // check for operations like "add rx, rx, 0" or "add r0, anything"
    if(isDstReg) {
      bool isDstR0 = i.dst.isReg() && i.dst.index_or_imm.has_value()  && i.dst.index_or_imm.value() == 0;
      bool areDstAndOp1Same = i.dst.isReg() && i.op1.isReg() && i.dst.index_or_imm.has_value() && i.op1.index_or_imm.has_value() && i.op1.index_or_imm.value() == i.dst.index_or_imm.value();
      bool areDstAndOp2Same = i.dst.isReg() && i.op2.isReg() && i.dst.index_or_imm.has_value() && i.op2.index_or_imm.has_value() && i.op2.index_or_imm.value() == i.dst.index_or_imm.value();
      if (isDstR0) continue;
      if (i.canDoDCE()) {
        if (areDstAndOp1Same) {
          if (i.op2.isImm() && i.op2.index_or_imm.value() == 0) continue;
        }

        if (areDstAndOp2Same) {
          if (i.op1.isImm() && i.op1.index_or_imm.value() == 0) continue;
        }
      }
    }

    code_.push_back(i);
  }
}

void IR::optimize() {
  std::vector<Entry> optimized{};

  dead_code_elimination(optimized);

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