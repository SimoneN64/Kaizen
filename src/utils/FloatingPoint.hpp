//
// Created by simone on 6/25/24.
//

#pragma once
#include <common.hpp>
#include <cmath>
#include <immintrin.h>

namespace Util {
template <typename T>
static inline T roundCeil(float f) {
#ifdef SIMD_SUPPORT
  __m128 t = _mm_set_ss(f);
  t = _mm_round_ss(t, t, _MM_FROUND_TO_POS_INF);
  return _mm_cvtss_f32(t);
#else
  return ceilf(f);
#endif
}

template <typename T>
static inline T roundCeil(double f) {
#ifdef SIMD_SUPPORT
  __m128d t = _mm_set_sd(f);
  t = _mm_round_sd(t, t, _MM_FROUND_TO_POS_INF);
  return _mm_cvtsd_f64(t);
#else
  return ceil(f);
#endif
}

template <typename T>
static inline T roundNearest(float f) {
#ifdef SIMD_SUPPORT
  __m128 t = _mm_set_ss(f);
  t = _mm_round_ss(t, t, _MM_FROUND_TO_NEAREST_INT);
  return _mm_cvtss_f32(t);
#else
  return roundf(f);
#endif
}

template <typename T>
static inline T roundNearest(double f) {
#ifdef SIMD_SUPPORT
  __m128d t = _mm_set_sd(f);
  t = _mm_round_sd(t, t, _MM_FROUND_TO_NEAREST_INT);
  return _mm_cvtsd_f64(t);
#else
  return round(f);
#endif
}

template<typename T>
static inline T roundCurrent(float f) {
#ifdef SIMD_SUPPORT
  auto t = _mm_set_ss(f);
  t = _mm_round_ss(t, t, _MM_FROUND_CUR_DIRECTION);
  return _mm_cvtss_f32(t);
#else
  return rint(f);
#endif
}

template<typename T>
static inline T roundCurrent(double f) {
#ifdef SIMD_SUPPORT
  auto t = _mm_set_sd(f);
  t = _mm_round_sd(t, t, _MM_FROUND_CUR_DIRECTION);
  return _mm_cvtsd_f64(t);
#else
  return rint(f);
#endif
}


template <typename T>
static inline T roundFloor(float f) {
#ifdef SIMD_SUPPORT
__m128 t = _mm_set_ss(f);
  t = _mm_round_ss(t, t, _MM_FROUND_TO_NEG_INF);
  return _mm_cvtss_f32(t);
#else
return floor(f);
#endif
}

template <typename T>
static inline T roundFloor(double f) {
#ifdef SIMD_SUPPORT
  __m128d t = _mm_set_sd(f);
  t = _mm_round_sd(t, t, _MM_FROUND_TO_NEG_INF);
  return _mm_cvtsd_f64(t);
#else
  return floor(f);
#endif
}

template <typename T>
static inline T roundTrunc(float f) {
#ifdef SIMD_SUPPORT
  __m128 t = _mm_set_ss(f);
  t = _mm_round_ss(t, t, _MM_FROUND_TO_ZERO);
  return _mm_cvtss_f32(t);
#else
  return trunc(f);
#endif
}

template <typename T>
static inline T roundTrunc(double f) {
#ifdef SIMD_SUPPORT
  __m128d t = _mm_set_sd(f);
  t = _mm_round_sd(t, t, _MM_FROUND_TO_ZERO);
  return _mm_cvtsd_f64(t);
#else
  return trunc(f);
#endif
}
}