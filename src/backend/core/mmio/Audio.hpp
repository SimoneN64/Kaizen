#pragma once
#include <MemoryHelpers.hpp>
#include <SDL3/SDL_audio.h>

namespace n64 {
struct AudioDevice {
  AudioDevice();
  ~AudioDevice();

  void Reset() { running = false; }

  void PushSample(float, float, float, float);
  void AdjustSampleRate(int);
  void LockMutex() {
    if (audioStreamMutex)
      SDL_LockMutex(audioStreamMutex);
  }
  void UnlockMutex() {
    if (audioStreamMutex)
      SDL_UnlockMutex(audioStreamMutex);
  }

  SDL_AudioStream *GetStream() { return audioStream; }

private:
  SDL_AudioStream *audioStream;
  SDL_Mutex *audioStreamMutex;
  SDL_AudioSpec request{};
  bool running = false;
};

} // namespace n64
