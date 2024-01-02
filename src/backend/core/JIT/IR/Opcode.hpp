#pragma once
#include <log.hpp>
#include <IR/Value.hpp>
#include <IR/Register.hpp>

namespace n64 {
enum class IROpcodeClass {
  StorePC, Add, Special, Regimm, COP0, COP1
};

struct IROpcode {
  virtual ~IROpcode() = default;
  virtual auto GetClass() const -> IROpcodeClass = 0;
  virtual auto Reads (IRVariable const& var) -> bool = 0;
  virtual auto Writes(IRVariable const& var) -> bool = 0;
  virtual void Repoint(IRVariable const& var_old, IRVariable const& var_new) = 0;
  virtual void PropagateConstant(IRVariable const& var, IRConstant const& constant) {}
  virtual auto ToString() -> std::string = 0;
};

template<IROpcodeClass _class>
struct IROpcodeBase : IROpcode {
  auto GetClass() const -> IROpcodeClass override { return _class; }
};

template<IROpcodeClass _class>
struct IRBinaryOpBase : IROpcodeBase<_class> {
  IRBinaryOpBase(IRVariable const& result, IRVariable lhs, IRAnyRef rhs)
    : result(const_cast<IRVariable &>(result)), lhs(lhs), rhs(rhs) {}

  IRVariable& result;
  IRVarRef lhs;
  IRAnyRef rhs;

  auto Reads(IRVariable const& var) -> bool override {
    return &lhs.Get() == &var ||
           (rhs.IsVariable() && (&rhs.GetVar() == &var));
  }

  auto Writes(IRVariable const& var) -> bool override {
    return result.HasValue() && (&result == &var);
  }

  void Repoint(
    IRVariable const& var_old,
    IRVariable const& var_new
  ) override {
    // TODO: make this reusable?
    if (result.HasValue() && (&result == &var_old)) {
      result = var_new;
    }

    lhs.Repoint(var_old, var_new);
    rhs.Repoint(var_old, var_new);
  }

  void PropagateConstant(
    IRVariable const& var,
    IRConstant const& constant
  ) override {
    rhs.PropagateConstant(var, constant);
  }
};

struct IRStorePC final : IROpcodeBase<IROpcodeClass::StorePC> {
  IRStorePC(IRAnyRef val) : val(val) {}

  IRAnyRef val;

  auto Reads(IRVariable const& var) -> bool override {
    if(val.IsVariable()) {
      return &var == &val.GetVar();
    }
    return false;
  }

  auto Writes(IRVariable const& var) -> bool override {
    return true;
  }

  void Repoint(
    IRVariable const& var_old,
    IRVariable const& var_new
  ) override {
  }

  auto ToString() -> std::string override {
    return fmt::format("str_pc {}", std::to_string(val));
  }
};

struct IRAdd final : IRBinaryOpBase<IROpcodeClass::Add> {
  using IRBinaryOpBase::IRBinaryOpBase;

  auto ToString() -> std::string override {
    return fmt::format(
      "add {}, {}, {}",
      std::to_string(result),
      std::to_string(lhs),
      std::to_string(rhs)
    );
  }
};
}