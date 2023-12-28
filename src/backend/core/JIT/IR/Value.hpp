#pragma once
#include <log.hpp>

namespace n64 {
enum IRPrimitive {
  Uint32, Sint32, Uint64, Sint32, Uint128, Sint128
};

struct IRVariable {
  IRVariable(IRPrimitive type, const u32 id, char const* const label) : type(type), id(id), label(label) {}
private:
  IRPrimitive type;
  const u32 id;
  char const* const label;
};

struct IRConstant {
  IRConstant() {}
  IRConstant(IRPrimitive type, u128 value) : type(type), value(value) {}
private:
  IRPrimitive type = Uint128;
  u128 value = 0;
};

struct IRAnyRef {
  IRAnyRef() {}
  IRAnyRef(IRVariable const& variable) : type(Type::Variable), var(&variable) {}
  IRAnyRef(IRConstant const& constant) : type(Type::Constant), constant(constant) {}

  auto operator=(IRAnyRef const& other) -> IRAnyRef& {
    type = other.type;
    if (IsConstant()) {
      constant = other.constant;
    } else {
      var = other.var;
    }
    return *this;
  }

  bool IsNull() const { return type == Type::Null; }
  bool IsVariable() const { return type == Type::Variable; }
  bool IsConstant() const { return type == Type::Constant; }

  auto GetVar() const -> IRVariable const& {
    if (!IsVariable()) {
      Util::panic("called GetVar() but value is a constant or null");
    }
    return *var;
  }

  auto GetConst() const -> IRConstant const& {
    if (!IsConstant()) {
      Util::panic("called GetConst() but value is a variable or null");
    }
    return constant;
  }

  void Repoint(IRVariable const& var_old, IRVariable const& var_new) {
    if (IsVariable() && (&GetVar() == &var_old)) {
      var = &var_new;
    }
  }

  void PropagateConstant(IRVariable const& var, IRConstant const& constant) {
    if (IsVariable() && (&GetVar() == &var)) {
      type = Type::Constant;
      this->constant = constant;
    }
  }
private:
  enum Type {
    Null, Variable, Constant
  };

  Type type;

  union {
    IRVariable const* var;
    IRConstant constant;
  };
};

struct IRVarRef {
  IRVarRef(IRVariable const& var) : p_var(&var) {}

  auto Get() const -> IRVariable const& {
    return *p_var;
  }

  void Repoint(IRVariable const& var_old, IRVariable const& var_new) {
    if (&var_old == p_var) {
      p_var = &var_new;
    }
  }
private:
  IRVariable const* p_var;
};
}