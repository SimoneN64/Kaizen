#pragma once
#include <common.hpp>
#include <ctime>
#include <discord_rpc.h>
#include <string>

namespace Util {
struct RPC {
  enum State {
    Idling,
    Playing,
    MovieReplay,
    Paused,
  };

  RPC() {
    Discord_Initialize("1049669178124148806", &handlers, 1, nullptr);
    startTimestamp = time(nullptr);
  }

  static RPC &GetInstance() {
    static RPC instance;
    return instance;
  }

  [[nodiscard]] FORCE_INLINE const State &GetState() const { return currentState; }

  FORCE_INLINE void Update(State state, const std::string &game = "", const std::string &movieName = "") {
    DiscordRichPresence presence{};
    std::string textState, textDetails;
    currentState = state == Paused ? currentState : state;

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
      switch (currentState) {
      case Playing:
        textDetails = "In-game";
        textState = "Playing \"" + game + "\"";
        break;
      case MovieReplay:
        textDetails = "In-game";
        textState = "Replaying movie \"" + movieName + "\" in \"" + game + "\"";
        break;
      default:;
      }
      textState += " (Paused)";
      break;
    }

    presence.details = textDetails.c_str();
    presence.state = textState.c_str();
    presence.startTimestamp = startTimestamp;
    presence.largeImageText = "Kaizen";
    presence.largeImageKey = "logo";
    Discord_UpdatePresence(&presence);
  }

  FORCE_INLINE void Clear() { Discord_ClearPresence(); }

private:
  DiscordEventHandlers handlers{};
  s64 startTimestamp{};
  State currentState;
};
} // namespace Util
