#pragma once
#include <log.hpp>

namespace n64 {
enum IRPrimitive {
  Uint32, Sint32, Uint64, Sint64
};

struct IRVariable {
  IRVariable(IRPrimitive type, const u32 id, char const* const label) : type(type), id(id), label(label), assigned(false) {}
  IRPrimitive type;
  u32 id;
  char const* label;
  bool assigned;

  bool HasValue() const { return assigned; }
  bool IsNull() const { return !assigned; }
};

struct IRConstant {
  IRConstant() {}
  IRConstant(IRPrimitive type, u64 value) : type(type), value(value) {}

  u64 value = 0;
private:
  IRPrimitive type = Uint64;
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


namespace std {
inline auto to_string(n64::IRPrimitive data_type) -> std::string {
  switch (data_type) {
    case n64::IRPrimitive::Uint32:
      return "u32";
    case n64::IRPrimitive::Sint32:
      return "s32";
    case n64::IRPrimitive::Uint64:
      return "u64";
    case n64::IRPrimitive::Sint64:
      return "s64";
    default:
      return "???";
  }
}

inline auto to_string(n64::IRVariable const &variable) -> std::string {
  if (variable.label) {
    return fmt::format("var{}_{}", variable.id, variable.label);
  }
  return fmt::format("var{}", variable.id);
}

inline auto to_string(n64::IRConstant const &constant) -> std::string {
  return fmt::format("0x{:0X}", constant.value);
}

inline auto to_string(n64::IRAnyRef const &value) -> std::string {
  if (value.IsNull()) {
    return "(null)";
  }
  if (value.IsConstant()) {
    return std::to_string(value.GetConst());
  }
  return std::to_string(value.GetVar());
}

inline auto to_string(n64::IRVarRef const &variable) -> std::string {
  return std::to_string(variable.Get());
}
}