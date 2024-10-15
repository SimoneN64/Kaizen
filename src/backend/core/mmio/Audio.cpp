#include <Audio.hpp>
#include <SDL3/SDL_audio.h>
#include <SDL3/SDL_init.h>
#include <log.hpp>

namespace n64 {
#define AUDIO_SAMPLE_RATE 44100
#define SYSTEM_SAMPLE_FORMAT SDL_AUDIO_F32
#define SYSTEM_SAMPLE_SIZE 4
#define BYTES_PER_HALF_SECOND (((float)AUDIO_SAMPLE_RATE / 2) * SYSTEM_SAMPLE_SIZE)

AudioDevice::~AudioDevice() {
  LockMutex();
  SDL_DestroyAudioStream(GetStream());
  UnlockMutex();
  SDL_DestroyMutex(audioStreamMutex);
}

AudioDevice::AudioDevice() {
  audioStreamMutex = SDL_CreateMutex();
  if (!audioStreamMutex) {
    Util::panic("Unable to initialize audio mutex: {}", SDL_GetError());
  }

  SDL_InitSubSystem(SDL_INIT_AUDIO);
  request = {SYSTEM_SAMPLE_FORMAT, 2, AUDIO_SAMPLE_RATE};

  audioStream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &request, nullptr, nullptr);
  if (!audioStream) {
    Util::panic("Unable to create audio stream: {}", SDL_GetError());
  }
}

void AudioDevice::PushSample(const float left, const float volumeL, const float right, const float volumeR) {
  const float adjustedL = left * volumeL;
  const float adjustedR = right * volumeR;
  const float samples[]{adjustedL, adjustedR};

  if (const auto availableBytes = static_cast<float>(SDL_GetAudioStreamAvailable(audioStream));
      availableBytes <= BYTES_PER_HALF_SECOND) {
    SDL_PutAudioStreamData(audioStream, samples, 2 * SYSTEM_SAMPLE_SIZE);
  }

  if (!running) {
    SDL_ResumeAudioStreamDevice(audioStream);
    running = true;
  }
}

void AudioDevice::AdjustSampleRate(int sampleRate) {
  LockMutex();
  SDL_DestroyAudioStream(audioStream);

  if (sampleRate < 4000) { // hack for Animal Forest. It requests a frequency of 3000-something. Weird asf
    sampleRate *= 4000.f / static_cast<float>(sampleRate);
  }
  request = {SYSTEM_SAMPLE_FORMAT, 2, sampleRate};

  audioStream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &request, nullptr, nullptr);
  if (!audioStream) {
    Util::panic("Unable to create audio stream: {}", SDL_GetError());
  }
  UnlockMutex();
}
} // namespace n64
