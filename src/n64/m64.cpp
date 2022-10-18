#include <m64.hpp>
#include <util.hpp>

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

union TASMovieControllerData {
  struct {
    bool dpad_right: 1;
    bool dpad_left: 1;
    bool dpad_down: 1;
    bool dpad_up: 1;
    bool start: 1;
    bool z: 1;
    bool b: 1;
    bool a: 1;
    bool c_right: 1;
    bool c_left: 1;
    bool c_down: 1;
    bool c_up: 1;
    bool r: 1;
    bool l: 1;
    u8: 2;
    s8 analog_x: 8;
    s8 analog_y: 8;
  };
  u32 raw;
} __attribute__((packed));

static_assert(sizeof(TASMovieControllerData) == 4);

static u8* loaded_tas_movie = nullptr;
static size_t loaded_tas_movie_size = 0;
TASMovieHeader loaded_tas_movie_header;
uint32_t loaded_tas_movie_index = 0;

void LoadTAS(const char* filename) {
  FILE *fp = fopen(filename, "rb");

  if (!fp) {
    util::panic("Error opening the movie file {}! Are you sure it's a valid movie and that it exists?", filename);
  }

  fseek(fp, 0, SEEK_END);
  size_t size = ftell(fp);

  fseek(fp, 0, SEEK_SET);
  u8 *buf = (u8*)malloc(size);
  fread(buf, size, 1, fp);

  loaded_tas_movie = buf;
  loaded_tas_movie_size = size;

  if (!loaded_tas_movie) {
    util::panic("Error loading movie!");
  }

  memcpy(&loaded_tas_movie_header, buf, sizeof(TASMovieHeader));

  if (loaded_tas_movie_header.signature[0] != 0x4D || loaded_tas_movie_header.signature[1] != 0x36 || loaded_tas_movie_header.signature[2] != 0x34 || loaded_tas_movie_header.signature[3] != 0x1A) {
    util::panic("Failed to load movie: incorrect signature. Are you sure this is a valid movie?");
  }

  if (loaded_tas_movie_header.version != 3) {
    util::panic("This movie is version {}: only version 3 is supported.", loaded_tas_movie_header.version);
  }

  if (loaded_tas_movie_header.startType != 2) {
    util::panic("Movie start type is {} - only movies with a start type of 2 are supported (start at power on)", loaded_tas_movie_header.startType);
  }

  // TODO: check ROM CRC32 here

  util::print("Loaded movie '{}' ", loaded_tas_movie_header.movie_description);
  util::print("by {}\n", loaded_tas_movie_header.author_name);
  util::print("{} controller(s) connected\n", loaded_tas_movie_header.numControllers);

  if (loaded_tas_movie_header.numControllers != 1) {
    util::panic("Currently, only movies with 1 controller connected are supported.\n");
  }

  loaded_tas_movie_index = sizeof(TASMovieHeader) - 4; // skip header
}

bool tas_movie_loaded() {
  return loaded_tas_movie != nullptr;
}

n64::Controller tas_next_inputs() {
  if (loaded_tas_movie_index + sizeof(TASMovieControllerData) > loaded_tas_movie_size) {
    loaded_tas_movie = nullptr;
    n64::Controller empty_controller{};
    memset(&empty_controller, 0, sizeof(n64::Controller));
    return empty_controller;
  }

  TASMovieControllerData movie_cdata{};
  memcpy(&movie_cdata, loaded_tas_movie + loaded_tas_movie_index, sizeof(TASMovieControllerData));

  loaded_tas_movie_index += sizeof(TASMovieControllerData);

  n64::Controller controller{};
  memset(&controller, 0, sizeof(controller));

  controller.c_right = movie_cdata.c_right;
  controller.c_left = movie_cdata.c_left;
  controller.c_down = movie_cdata.c_down;
  controller.c_up = movie_cdata.c_up;
  controller.r = movie_cdata.r;
  controller.l = movie_cdata.l;

  controller.dp_right = movie_cdata.dpad_right;
  controller.dp_left = movie_cdata.dpad_left;
  controller.dp_down = movie_cdata.dpad_down;
  controller.dp_up = movie_cdata.dpad_up;

  controller.z = movie_cdata.z;
  controller.b = movie_cdata.b;
  controller.a = movie_cdata.a;
  if(movie_cdata.start) {
    printf("\n");
  }
  controller.start = movie_cdata.start;

  controller.joy_x = movie_cdata.analog_x;
  controller.joy_y = movie_cdata.analog_y;

  return controller;
}