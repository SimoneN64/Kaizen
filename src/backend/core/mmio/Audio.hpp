#pragma once
#include <common.hpp>
#include <SDL2/SDL.h>

namespace n64 {
struct AudioDevice {
  AudioDevice();
  ~AudioDevice() {
    SDL_FreeAudioStream(audioStream);
    SDL_DestroyMutex(audioStreamMutex);
  }

  void PushSample(float, float, float, float);
  void AdjustSampleRate(int);
  void LockMutex() {
    if(audioStreamMutex)
      SDL_LockMutex(audioStreamMutex);
  }
  void UnlockMutex() {
    if (audioStreamMutex)
      SDL_UnlockMutex(audioStreamMutex);
  }

  SDL_AudioStream* GetStream() { return audioStream; }
private:
  SDL_AudioStream* audioStream = nullptr;
  SDL_mutex* audioStreamMutex = nullptr;
  SDL_AudioSpec audioSpec{};
  SDL_AudioSpec request{};
  SDL_AudioDeviceID handle{};
};

}