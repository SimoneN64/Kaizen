#pragma once
#include <backend/core/mmio/PIF.hpp>

void LoadTAS(const char* filename);
void UnloadTAS();
n64::Controller tas_next_inputs();
bool tas_movie_loaded();