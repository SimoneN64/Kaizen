#pragma once
#include <backend/core/mmio/PIF.hpp>

void LoadTAS(const char* filename);
void UnloadTAS();
n64::Controller TasNextInputs();
bool TasMovieLoaded();