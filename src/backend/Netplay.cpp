#include <Netplay.hpp>
#include "log.hpp"
#include "PIF/Device.hpp"
#include <PIF.hpp>
#include <array>

namespace Netplay {
bool isHost = false;
bool connected = false;
std::array<std::string, 4> IPs{};

template <int port>
n64::JoybusDevice RequestDataFromRemotePlayer() {
  sf::UdpSocket sock;
  u16 remotePort = 53000;
  sf::IpAddress remoteAddress = IPs[port];
  if(sock.bind(remotePort) != sf::Socket::Done) {
    Util::panic("Could not bind UDP connection");
  }

  sf::Packet res;
  if(sock.receive(res, remoteAddress, remotePort) != sf::Socket::Done) {
    Util::panic("Could not receive data from other players");
  }

  n64::JoybusDevice result{};
  result << res;
}

template <int port>
void SendDataToRemotePlayer() {

}

template <int port>
void SyncPlayers(n64::PIF& pif) {
  if(connected) {
    if(isHost) {
      pif.PollController();
    } else {
      if constexpr (port == 0) {
        Util::panic("It shouldn't be possible to be player 1 and not be the host");
      } else {
        if constexpr (port > 3) {
          Util::panic("What the heeeeelll oh my gaaaa");
        }

        n64::players[port] = RequestDataFromRemotePlayer<port>();
      }
    }
  }
}

template void SyncPlayers(n64::PIF&);
}