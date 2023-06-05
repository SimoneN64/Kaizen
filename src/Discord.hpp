#pragma once
#include <discord_rpc.h>
#include <string>
#include <ctime>

namespace Util {
enum State {
  Idling,
  Playing,
  Paused
};

FORCE_INLINE void UpdateRPC(State state, const std::string& game = "") {
  DiscordRichPresence presence{};
  std::string textState, textDetails;

  switch(state) {
    case Idling:
      textDetails = "Idling";
      break;
    case Playing:
      textDetails = "In-game";
      textState = "Playing \"" + game + "\"";
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

FORCE_INLINE void ClearRPC() {
  Discord_ClearPresence();
}
}