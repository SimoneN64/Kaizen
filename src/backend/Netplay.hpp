#pragma once
#include <SFML/Network.hpp>
#include <PIF.hpp>

namespace Netplay {
extern bool isHost;
extern bool connected;

template <int port = 0>
void SyncPlayers(n64::PIF&);
}