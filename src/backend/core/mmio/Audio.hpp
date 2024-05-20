#pragma once
#include <MemoryHelpers.hpp>
#include <SDL2/SDL.h>

namespace n64 {
struct AudioDevice {
  AudioDevice();

  void PushSample(float, float, float, float);
  void AdjustSampleRate(int);
  void LockMutex() {
    if(audioStreamMutex.get())
      SDL_LockMutex(audioStreamMutex.get());
  }
  void UnlockMutex() {
    if (audioStreamMutex.get())
      SDL_UnlockMutex(audioStreamMutex.get());
  }

  Util::AutoRelease<SDL_AudioStream, const SDL_AudioFormat, const Uint8, const int, const SDL_AudioFormat,
                    const Uint8, const int>& GetStream() { return audioStream; }
private:
  Util::AutoRelease<SDL_AudioStream, const SDL_AudioFormat, const Uint8, const int, const SDL_AudioFormat,
                  const Uint8, const int> audioStream;
  Util::AutoRelease<SDL_mutex> audioStreamMutex;
  SDL_AudioSpec audioSpec{};
  SDL_AudioSpec request{};
  SDL_AudioDeviceID handle{};
};

}