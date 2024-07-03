#pragma once
#include <MemoryHelpers.hpp>
#include <SDL2/SDL_audio.h>

namespace n64 {
struct AudioDevice {
  AudioDevice();
  ~AudioDevice();

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
  SDL_AudioStream* audioStream;
  SDL_mutex* audioStreamMutex;
  SDL_AudioSpec audioSpec{};
  SDL_AudioSpec request{};
  SDL_AudioDeviceID handle{};
};

}