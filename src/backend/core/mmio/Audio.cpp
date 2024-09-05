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

void AudioDevice::PushSample(float left, float volumeL, float right, float volumeR) {
  float adjustedL = left * volumeL;
  float adjustedR = right * volumeR;
  float samples[]{adjustedL, adjustedR};

  auto availableBytes = (float)SDL_GetAudioStreamAvailable(audioStream);
  if (availableBytes <= BYTES_PER_HALF_SECOND) {
    SDL_PutAudioStreamData(audioStream, samples, 2 * sizeof(float));
  }

  if (!running) {
    SDL_ResumeAudioStreamDevice(audioStream);
    running = true;
  }
}

void AudioDevice::AdjustSampleRate(int sampleRate) {
  LockMutex();
  SDL_DestroyAudioStream(audioStream);

  request = {SYSTEM_SAMPLE_FORMAT, 2, sampleRate};

  audioStream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &request, nullptr, nullptr);
  if (!audioStream) {
    Util::panic("Unable to create audio stream: {}", SDL_GetError());
  }
  UnlockMutex();
}
} // namespace n64
