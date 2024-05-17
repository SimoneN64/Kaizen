#include <Audio.hpp>
#include <log.hpp>

namespace n64 {
#define AUDIO_SAMPLE_RATE 44100
#define SYSTEM_SAMPLE_FORMAT AUDIO_F32SYS
#define SYSTEM_SAMPLE_SIZE 4
#define BYTES_PER_HALF_SECOND (((float)AUDIO_SAMPLE_RATE / 2) * SYSTEM_SAMPLE_SIZE)

void audioCallback(void* user, Uint8* stream, int length) {
  auto audioDevice = (AudioDevice*)user;
  int gotten = 0;
  audioDevice->LockMutex();
  int available = SDL_AudioStreamAvailable(audioDevice->GetStream());

  if (available > 0) {
    gotten = SDL_AudioStreamGet(audioDevice->GetStream(), stream, length);
  }
  audioDevice->UnlockMutex();

  int gottenSamples = (int)(gotten / sizeof(float));
  auto* out = (float*)stream;
  out += gottenSamples;

  for (int i = gottenSamples; i < length / sizeof(float); i++) {
    float sample = 0;
    *out++ = sample;
  }
}

AudioDevice::AudioDevice() {
  SDL_InitSubSystem(SDL_INIT_AUDIO);
  AdjustSampleRate(AUDIO_SAMPLE_RATE);

  audioStreamMutex = SDL_CreateMutex();

  request.freq = AUDIO_SAMPLE_RATE;
  request.format = SYSTEM_SAMPLE_FORMAT;
  request.channels = 2;
  request.samples = 1024;
  request.callback = audioCallback;
  request.userdata = (void*)this;

  if(!handle) {
    handle = SDL_OpenAudioDevice(nullptr, 0, &request, &audioSpec, 0);
  }

  if(!handle) {
    Util::panic("Failed to initialize SDL Audio: {}", SDL_GetError());
  }

  SDL_PauseAudioDevice(handle, false);

  if(!audioStreamMutex) {
    Util::panic("Unable to initialize audio mutex: {}", SDL_GetError());
  }
}

void AudioDevice::PushSample(float left, float volumeL, float right, float volumeR) {
  float adjustedL = left * volumeL;
  float adjustedR = right * volumeR;
  float samples[2]{ adjustedL, adjustedR };

  float availableBytes = SDL_AudioStreamAvailable(audioStream);
  if(availableBytes <= BYTES_PER_HALF_SECOND) {
    SDL_AudioStreamPut(audioStream, samples, 2 * sizeof(float));
  }
}

void AudioDevice::AdjustSampleRate(int sampleRate) {
  LockMutex();
  if(audioStream) SDL_FreeAudioStream(audioStream);

  audioStream = SDL_NewAudioStream(AUDIO_F32SYS, 2, sampleRate, SYSTEM_SAMPLE_FORMAT, 2, AUDIO_SAMPLE_RATE);
  UnlockMutex();
}
}