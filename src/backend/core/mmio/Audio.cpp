#include <Audio.hpp>
#include <log.hpp>
#include <SDL2/SDL.h>

namespace n64 {
#define AUDIO_SAMPLE_RATE 44100
#define SYSTEM_SAMPLE_FORMAT AUDIO_F32SYS
#define SYSTEM_SAMPLE_SIZE 4
#define BYTES_PER_HALF_SECOND (((float)AUDIO_SAMPLE_RATE / 2) * SYSTEM_SAMPLE_SIZE)

void audioCallback(void* user, Uint8* stream, int length) {
  auto audioDevice = (AudioDevice*)user;
  int gotten = 0, available = 0;
  if (audioDevice) {
    audioDevice->LockMutex();
  }

  if (audioDevice) {
    available = SDL_AudioStreamAvailable(audioDevice->GetStream().get());
  }

  if (available > 0 && audioDevice) {
    gotten = SDL_AudioStreamGet(audioDevice->GetStream().get(), stream, length);
  }

  if (audioDevice) {
    audioDevice->UnlockMutex();
  }

  int gottenSamples = (int)(gotten / sizeof(float));
  auto* out = (float*)stream;
  out += gottenSamples;

  for (int i = gottenSamples; i < length / sizeof(float); i++) {
    float sample = 0;
    *out++ = sample;
  }
}

AudioDevice::AudioDevice() : audioStream(SDL_NewAudioStream, SDL_FreeAudioStream, "audioStream", SYSTEM_SAMPLE_FORMAT, 2, AUDIO_SAMPLE_RATE, SYSTEM_SAMPLE_FORMAT, 2, AUDIO_SAMPLE_RATE),
                             audioStreamMutex(SDL_CreateMutex, SDL_DestroyMutex, "audioStreamMutex") {
  SDL_InitSubSystem(SDL_INIT_AUDIO);

  if(!audioStreamMutex.get()) {
    Util::panic("Unable to initialize audio mutex: {}", SDL_GetError());
  }

  request.freq = AUDIO_SAMPLE_RATE;
  request.format = SYSTEM_SAMPLE_FORMAT;
  request.channels = 2;
  request.samples = 1024;
  request.callback = audioCallback;
  request.userdata = (void*)this;

  handle = SDL_OpenAudioDevice(nullptr, 0, &request, &audioSpec, 0);

  if(!handle) {
    Util::panic("Failed to initialize SDL Audio: {}", SDL_GetError());
  }

  SDL_PauseAudioDevice(handle, false);
}

void AudioDevice::PushSample(float left, float volumeL, float right, float volumeR) {
  float adjustedL = left * volumeL;
  float adjustedR = right * volumeR;
  float samples[2]{ adjustedL, adjustedR };

  auto availableBytes = (float)SDL_AudioStreamAvailable(audioStream.get());
  if(availableBytes <= BYTES_PER_HALF_SECOND) {
    SDL_AudioStreamPut(audioStream.get(), samples, 2 * sizeof(float));
  }
}

void AudioDevice::AdjustSampleRate(int sampleRate) {
  LockMutex();
  audioStream.Construct(SDL_NewAudioStream, SYSTEM_SAMPLE_FORMAT, 2, sampleRate, SYSTEM_SAMPLE_FORMAT, 2, AUDIO_SAMPLE_RATE);
  UnlockMutex();
}
}