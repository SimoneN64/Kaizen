#pragma once
#include <log.hpp>

namespace n64 {
enum class IROpcodeClass {
  Normal, Special, Regimm, COP0, COP1
};

struct IROpcode {
  virtual ~IROpcode() = default;
  virtual auto GetClass() const -> IROpcodeClass = 0;
  virtual auto Reads (IRVariable const& var) -> bool = 0;
  virtual auto Writes(IRVariable const& var) -> bool = 0;
  virtual void Repoint(IRVariable const& var_old, IRVariable const& var_new) = 0;
  virtual void PropagateConstant(IRVariable const& var, IRConstant const& constant) {}
};

template<IROpcodeClass _class>
struct IROpcodeBase : IROpcode {
  auto GetClass() const -> IROpcodeClass override { return _class; }
};

struct IRNoOp final : IROpcodeBase<IROpcodeClass::NOP> {
  auto Reads(IRVariable const& var) -> bool override {
    return false;
  }

  auto Writes(IRVariable const& var) -> bool override {
    return false;
  }

  void Repoint(
    IRVariable const& var_old,
    IRVariable const& var_new
  ) override {
  }

  auto ToString() -> std::string override {
    return "nop";
  }
};
}