#pragma once
#include <common.hpp>
#include <ctime>
#include <discord_rpc.h>
#include <string>

namespace Util {
enum State {
  Idling,
  Playing,
  MovieReplay,
  Paused,
};

FORCE_INLINE void UpdateRPC(State state, const std::string &game = "", const std::string &movieName = "") {
  DiscordRichPresence presence{};
  std::string textState, textDetails;

  switch (state) {
  case Idling:
    textDetails = "Idling";
    break;
  case Playing:
    textDetails = "In-game";
    textState = "Playing \"" + game + "\"";
    break;
  case MovieReplay:
    textDetails = "In-game";
    textState = "Replaying movie \"" + movieName + "\" in \"" + game + "\"";
    break;
  case Paused:
    textDetails = "In-game";
    textState = "Playing \"" + game + "\" (Paused)";
    break;
  }

  presence.details = textDetails.c_str();
  presence.state = textState.c_str();
  presence.startTimestamp = time(nullptr);
  presence.largeImageText = "Kaizen";
  presence.largeImageKey = "logo";
  Discord_UpdatePresence(&presence);
}

FORCE_INLINE void ClearRPC() { Discord_ClearPresence(); }
} // namespace Util
