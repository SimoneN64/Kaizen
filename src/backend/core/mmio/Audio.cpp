#include <Audio.hpp>
#include <SDL3/SDL_audio.h>
#include <SDL3/SDL_init.h>
#include <log.hpp>

namespace n64 {
#define AUDIO_SAMPLE_RATE 44100
#define SYSTEM_SAMPLE_FORMAT SDL_AUDIO_F32
#define SYSTEM_SAMPLE_SIZE 4
#define BYTES_PER_HALF_SECOND (((float)AUDIO_SAMPLE_RATE / 2) * SYSTEM_SAMPLE_SIZE)

void audioCallback(void *userdata, SDL_AudioStream *stream, int additional_amount, int total_amount) {
  auto audioDevice = (AudioDevice *)userdata;
  int gotten = 0, available = 0;

  if (audioDevice) {
    audioDevice->LockMutex();
    available = SDL_GetAudioStreamAvailable(audioDevice->GetStream());
    if (available > 0) {
      gotten = SDL_GetAudioStreamData(audioDevice->GetStream(), stream, total_amount);
    }
    audioDevice->UnlockMutex();
  }

  int gottenSamples = (int)(gotten / sizeof(float));
  auto *out = (float *)stream;
  out += gottenSamples;

  for (int i = gottenSamples; i < total_amount / sizeof(float); i++) {
    float sample = 0;
    *out++ = sample;
  }
}

AudioDevice::~AudioDevice() {
  LockMutex();
  SDL_DestroyAudioStream(GetStream());
  UnlockMutex();
  SDL_DestroyMutex(audioStreamMutex);
}

AudioDevice::AudioDevice() : audioStreamMutex(SDL_CreateMutex()) {
  request.freq = AUDIO_SAMPLE_RATE;
  request.format = SYSTEM_SAMPLE_FORMAT;
  request.channels = 2;

  audioStream = SDL_CreateAudioStream(&request, &request);

  SDL_InitSubSystem(SDL_INIT_AUDIO);

  if (!audioStreamMutex) {
    Util::panic("Unable to initialize audio mutex: {}", SDL_GetError());
  }

  handle = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &request);
  SDL_BindAudioStream(handle, audioStream);
  SDL_SetAudioStreamGetCallback(audioStream, audioCallback, this);

  if (!handle) {
    Util::panic("Failed to initialize SDL Audio: {}", SDL_GetError());
  }

  SDL_PauseAudioDevice(handle);
}

void AudioDevice::PushSample(float left, float volumeL, float right, float volumeR) {
  float adjustedL = left * volumeL;
  float adjustedR = right * volumeR;
  float samples[2]{adjustedL, adjustedR};

  auto availableBytes = (float)SDL_GetAudioStreamAvailable(audioStream);
  if (availableBytes <= BYTES_PER_HALF_SECOND) {
    SDL_PutAudioStreamData(audioStream, samples, 2 * sizeof(float));
  }
}

void AudioDevice::AdjustSampleRate(int sampleRate) {
  LockMutex();
  SDL_DestroyAudioStream(audioStream);

  auto oldReq = request;

  request.freq = sampleRate;
  request.format = SYSTEM_SAMPLE_FORMAT;
  request.channels = 2;

  audioStream = SDL_CreateAudioStream(&request, &oldReq);
  UnlockMutex();
}
} // namespace n64
