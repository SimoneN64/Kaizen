#pragma once
#include <common.hpp>
#include <cmath>
#include <cfenv>

struct f32 {
  f32() = default;
  explicit f32(float v) : val(v) {}

  bool is_quiet() {
    return is_nan() && !is_signaling();
  }

  f32& operator=(const f32& rhs) = default;
  f32& operator=(const float& rhs) {
    val = rhs;
    return *this;
  }

  f32 operator+(const f32& rhs) const {
    return f32(val + rhs.val);
  }
  f32 operator-(const f32& rhs) const {
    return f32(val - rhs.val);
  }
  f32 operator/(const f32& rhs) const {
    return f32(val / rhs.val);
  }
  f32 operator*(const f32& rhs) const {
    return f32(val * rhs.val);
  }

  bool operator>=(const float& rhs) const {
    return val >= rhs;
  }
  bool operator<=(const float& rhs) const {
    return val <= rhs;
  }
  bool operator<(const float& rhs) const {
    return val < rhs;
  }
  bool operator<(const f32& rhs) const {
    return val < rhs.val;
  }
  bool operator==(const f32& rhs) const {
    return val == rhs.val;
  }
  bool operator<=(const f32& rhs) const {
    return val <= rhs.val;
  }

  [[nodiscard]] bool is_nan() const {
    return std::isnan(val);
  }

  bool is_signaling() {
    static u32 QUIET_BIT = 0x00400000;
    return is_nan() && ((to_bits() & QUIET_BIT) == 0);
  }

  u32 to_bits() {
    return *(u32*)&val;
  }

  bool is_infinite() {
    return std::isinf(val);
  }

  bool is_subnormal() {
    return std::fpclassify(val) == FP_SUBNORMAL;
  }

  explicit operator float() const {
    return val;
  }
private:
  friend struct f32;
  float val{};
};

struct f64 {
  f64() = default;
  explicit f64(double v) : val(v) {}

  bool is_quiet() {
    return is_nan() && !is_signaling();
  }

  f64& operator=(const f64& rhs) = default;
  f64& operator=(const double& rhs) {
     val = rhs;
     return *this;
  }

  f64 operator+(const f64& rhs) const {
    return f64(val + rhs.val);
  }
  f64 operator-(const f64& rhs) const {
    return f64(val - rhs.val);
  }
  f64 operator/(const f64& rhs) const {
    return f64(val / rhs.val);
  }
  f64 operator*(const f64& rhs) const {
    return f64(val * rhs.val);
  }


  bool operator>=(const double& rhs) const {
    return val >= rhs;
  }
  bool operator<=(const double& rhs) const {
    return val <= rhs;
  }
  bool operator<(const double& rhs) const {
    return val < rhs;
  }
  bool operator<(const f64& rhs) const {
    return val < rhs.val;
  }
  bool operator==(const f64& rhs) const {
    return val == rhs.val;
  }
  bool operator<=(const f64& rhs) const {
    return val <= rhs.val;
  }

  [[nodiscard]] bool is_nan() const {
    return std::isnan(val);
  }

  bool is_subnormal() {
    return std::fpclassify(val) == FP_SUBNORMAL;
  }

  bool is_infinite() {
    return std::isinf(val);
  }

  bool is_signaling() {
    static u64 QUIET_BIT = 0x0008000000000000;
    return is_nan() && ((to_bits() & QUIET_BIT) == 0);
  }

  u64 to_bits() {
    return *(u64*)&val;
  }

  explicit operator double() const {
    return val;
  }
private:
  friend struct f64;
  double val{};
};

concept AnyFloat = requires(T a) {
  std::is_same_v<T, f32> || std::is_same_v<T, f64>;
};