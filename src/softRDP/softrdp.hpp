//
// Created by simone on 7/8/24.
//

#pragma once
#include <cstddef>

namespace Util {
template<size_t i, size_t d>
struct Fixed {

private:
  static constexpr float factor = 1.f/(10.f*d);
  float raw = 0.f;
};
}
