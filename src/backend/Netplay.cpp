#include <Netplay.hpp>
#include "log.hpp"
#include <PIF.hpp>

namespace Netplay {
bool isHost = false;
bool connected = false;
n64::Controller players[4]{};

template <int port>
void SyncPlayers(n64::PIF& pif) {
  if(connected) {
    if(isHost) {
      pif.PollController();
    } else {
      if constexpr (port == 0) {
        Util::panic("It shouldn't be possible to be player 1 and not be the host");
      } else if constexpr (port == 1) {

      } else if constexpr (port == 2) {

      } else if constexpr (port == 3) {

      } else {
        Util::panic("What the heeeeelll oh my gaaaa");
      }
    }
  }
}

template void SyncPlayers(n64::PIF&);
}