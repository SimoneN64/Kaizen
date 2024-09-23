#include <Disassembler.hpp>

Disassembler::DisassemblyResult Disassembler::DisassembleSimple(u32 address, u32 instruction) {
  cs_insn *insn;
  auto bytes = Util::IntegralToBuffer(instruction);
  auto count = cs_disasm(handle, bytes.data(), bytes.size(), address, 0, &insn);

  if (count <= 0)
    return {};

  DisassemblyResult result{true, fmt::format("0x{:016X}:\t{}\t{}", insn[0].address, insn[0].mnemonic, insn[0].op_str)};

  cs_free(insn, count);

  return result;
}

Disassembler::DisassemblyResult Disassembler::DisassembleDetailed(u32 address, u32 instruction) {
  cs_insn *insn;
  auto bytes = Util::IntegralToBuffer(instruction);
  auto count = cs_disasm(handle, bytes.data(), bytes.size(), address, 0, &insn);

  if (count <= 0)
    return {};

  DisassemblyResult result{true};
  result.address = insn[0].address;
  result.mnemonic = insn[0].mnemonic;

  result.full += result.address + ":\t";
  result.full += result.mnemonic + "\t";

  cs_detail *details = insn[0].detail;
  auto formatOperand = [&](const cs_mips_op &operand) {
    switch (operand.type) {
    case MIPS_OP_IMM:
      return fmt::format("#{:X}", operand.is_unsigned ? operand.uimm : operand.imm);
    case MIPS_OP_MEM:
      return fmt::format("{}(0x{:X})", cs_reg_name(handle, operand.mem.base), operand.mem.disp);
    case MIPS_OP_REG:
      return fmt::format("{}", cs_reg_name(handle, operand.reg));
    }
  };

  for (u8 i = 0; i < details->mips.op_count && i < 3; i++) {
    result.ops[i] = formatOperand(details->mips.operands[i]);
    result.full += result.ops[i] + "\t";
  }

  result.full += "\t// ";

  // TODO: generate a comment

  cs_free(insn, count);

  return result;
}
