#pragma once
#include <common.hpp>
#include <filesystem>
#include <vector>

namespace fs = std::filesystem;

namespace n64 {
struct Controller;
}

struct TASMovieHeader {
  u8 signature[4];
  u32 version;
  u32 uid;
  u32 numFrames;
  u32 rerecords;
  u8 fps;
  u8 numControllers;
  u8 reserved1;
  u8 reserved2;
  u32 numInputSamples;
  uint16_t startType;
  u8 reserved3;
  u8 reserved4;
  u32 controllerFlags;
  u8 reserved5[160];
  char romName[32];
  u32 romCrc32;
  uint16_t romCountryCode;
  u8 reserved6[56];
  // 122 64-byte ASCII string: name of video plugin used when recording, directly from plugin
  char video_plugin_name[64];
  // 162 64-byte ASCII string: name of sound plugin used when recording, directly from plugin
  char audio_plugin_name[64];
  // 1A2 64-byte ASCII string: name of input plugin used when recording, directly from plugin
  char input_plugin_name[64];
  // 1E2 64-byte ASCII string: name of rsp plugin used when recording, directly from plugin
  char rsp_plugin_name[64];
  // 222 222-byte UTF-8 string: author name info
  char author_name[222];
  // 300 256-byte UTF-8 string: author movie description info
  char movie_description[256];
} __attribute__((packed));

static_assert(sizeof(TASMovieHeader) == 1024);

struct MupenMovie {
  MupenMovie() = default;
  MupenMovie(const fs::path&);
  bool Load(const fs::path&);
  n64::Controller NextInputs();
  bool IsLoaded() const { return !loadedTasMovie.empty(); }
private:
  std::string filename = "";
  std::string game = "";
  std::vector<u8> loadedTasMovie = {};
  TASMovieHeader loadedTasMovieHeader = {};
  uint32_t loadedTasMovieIndex = 0;
};