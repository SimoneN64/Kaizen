#include <PIF/MupenMovie.hpp>
#include <log.hpp>
#include "File.hpp"
#include "PIF.hpp"

union TASMovieControllerData {
  struct {
    unsigned dpadRight : 1;
    unsigned dpadLeft : 1;
    unsigned dpadDown : 1;
    unsigned dpadUp : 1;
    unsigned start : 1;
    unsigned z : 1;
    unsigned b : 1;
    unsigned a : 1;
    unsigned cRight : 1;
    unsigned cLeft : 1;
    unsigned cDown : 1;
    unsigned cUp : 1;
    unsigned r : 1;
    unsigned l : 1;
    unsigned : 2;
    signed analogX : 8;
    signed analogY : 8;
  };
  u32 raw;
} __attribute__((packed));

static_assert(sizeof(TASMovieControllerData) == 4);

bool MupenMovie::Load(const fs::path &path) {
  loadedTasMovie = Util::ReadFileBinary(path.string());
  if (!IsLoaded()) {
    Util::error("Error loading movie!");
    return false;
  }

  memcpy(&loadedTasMovieHeader, loadedTasMovie.data(), sizeof(TASMovieHeader));

  if (loadedTasMovieHeader.signature[0] != 0x4D || loadedTasMovieHeader.signature[1] != 0x36 ||
      loadedTasMovieHeader.signature[2] != 0x34 || loadedTasMovieHeader.signature[3] != 0x1A) {
    Util::error("Failed to load movie: incorrect signature. Are you sure this is a valid movie?");
    return false;
  }

  if (loadedTasMovieHeader.version != 3) {
    Util::error("This movie is version {}: only version 3 is supported.", loadedTasMovieHeader.version);
    return false;
  }

  if (loadedTasMovieHeader.startType != 2) {
    Util::error("Movie start type is {} - only movies with a start type of 2 are supported (start at power on)",
                loadedTasMovieHeader.startType);
    return false;
  }

  Util::info("Loaded movie '{}' ", loadedTasMovieHeader.movie_description);
  Util::info("by {}", loadedTasMovieHeader.author_name);
  Util::info("{} controller(s) connected", loadedTasMovieHeader.numControllers);

  if (loadedTasMovieHeader.numControllers != 1) {
    Util::error("Currently, only movies with 1 controller connected are supported.");
    return false;
  }

  loadedTasMovieIndex = sizeof(TASMovieHeader) - 4; // skip header
  return true;
}

MupenMovie::MupenMovie(const fs::path &path) {
  if (!Load(path)) {
    Util::panic("");
  }
}

void MupenMovie::Reset() {
  if (!IsLoaded())
    return;

  loadedTasMovieIndex = sizeof(TASMovieHeader) - 4; // skip header
}

FORCE_INLINE void LogController(const n64::Controller &controller) {
  Util::debug("c_right: {}", controller.cRight);
  Util::debug("c_left: {}", controller.cLeft);
  Util::debug("c_down: {}", controller.cDown);
  Util::debug("c_up: {}", controller.cUp);
  Util::debug("r: {}", controller.r);
  Util::debug("l: {}", controller.l);
  Util::debug("dp_right: {}", controller.dpRight);
  Util::debug("dp_left: {}", controller.dpLeft);
  Util::debug("dp_down: {}", controller.dpDown);
  Util::debug("dp_up: {}", controller.dpUp);
  Util::debug("z: {}", controller.z);
  Util::debug("b: {}", controller.b);
  Util::debug("a: {}", controller.a);
  Util::debug("start: {}", controller.start);
  Util::debug("joy_x: {}", controller.joyX);
  Util::debug("joy_y: {}", controller.joyY);
}

n64::Controller MupenMovie::NextInputs() {
  if (loadedTasMovieIndex + sizeof(TASMovieControllerData) > loadedTasMovie.size()) {
    loadedTasMovie.clear();
    n64::Controller emptyController{};
    return emptyController;
  }

  TASMovieControllerData movieCData{};
  memcpy(&movieCData, &loadedTasMovie[loadedTasMovieIndex], sizeof(TASMovieControllerData));

  loadedTasMovieIndex += sizeof(TASMovieControllerData);

  n64::Controller controller{};

  controller.cRight = movieCData.cRight;
  controller.cLeft = movieCData.cLeft;
  controller.cDown = movieCData.cDown;
  controller.cUp = movieCData.cUp;
  controller.r = movieCData.r;
  controller.l = movieCData.l;

  controller.dpRight = movieCData.dpadRight;
  controller.dpLeft = movieCData.dpadLeft;
  controller.dpDown = movieCData.dpadDown;
  controller.dpUp = movieCData.dpadUp;

  controller.z = movieCData.z;
  controller.b = movieCData.b;
  controller.a = movieCData.a;
  controller.start = movieCData.start;

  controller.joyX = movieCData.analogX;
  controller.joyY = movieCData.analogY;

  LogController(controller);

  return controller;
}
