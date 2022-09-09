#pragma once
#include "common.hpp"

namespace n64 {
void PushSample(s16, s16);
void InitAudio();
void AdjustSampleRate(int);
}