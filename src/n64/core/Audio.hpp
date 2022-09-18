#pragma once
#include "common.hpp"

namespace n64 {
void PushSample(float, float, float, float);
void InitAudio();
void AdjustSampleRate(int);
}