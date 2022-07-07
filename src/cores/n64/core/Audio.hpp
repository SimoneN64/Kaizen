#pragma once
#include "common.hpp"

namespace natsukashii::core {
void PushSample(s16, s16);
void InitAudio();
void AdjustSampleRate(int);
}