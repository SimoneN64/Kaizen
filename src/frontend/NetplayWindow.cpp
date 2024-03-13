#define ENET_IMPLEMENTATION
#include <enet.h>
#include <NetplayWindow.hpp>
#include <QCloseEvent>
#include <QHideEvent>
#include <thread>
#include <log.hpp>
#include <arena.hpp>

enum PeerCommand : uint8_t {
  ePC_None,
  ePC_PeerList,
  ePC_NewPeer,
  ePC_JoinLobby,
  ePC_CreateLobby,
  ePC_Ping,
  ePC_Pong,
};

static std::vector<ENetPeer*> g_remote_peers;

void NetplayWindow::showEvent(QShowEvent *event) {
  disconnect = false;
  std::thread findLobbies([&]() {
    if (enet_initialize() != 0) {
      Util::println("An error occurred while initializing ENet.");
      return;
    }

    ENetAddress hostAddress;
    hostAddress.host = ENET_HOST_ANY;
    hostAddress.port = ENET_PORT_ANY;
    ENetHost* host = enet_host_create(&hostAddress, 4, 2, 0, 0);
    if (!host) {
      Util::println("An error occurred while creating the host.");
      return;
    }

    ENetAddress serverAddress;
    enet_address_set_host(&serverAddress, "gadolinium.dev");
    serverAddress.port = 7788;
    ENetPeer* serverPeer = enet_host_connect(host, &serverAddress, 2, 0);
    if(!serverPeer) {
      Util::println("An error occurred while connecting.");
      return;
    }

    ArenaBuffer wb;
    while(!disconnect) {
      ENetEvent evt;
      if(enet_host_service(host, &evt, 1000) > 0) {
        char ip[40];
        enet_address_get_host_ip(&evt.peer->address, ip, 40);

        if(evt.type == ENET_EVENT_TYPE_CONNECT) {
          Util::println("Connected to {}:{}", ip, evt.peer->address.port);
        } else if(evt.type == ENET_EVENT_TYPE_DISCONNECT) {
          Util::println("Disconnected to {}:{}", ip, evt.peer->address.port);

          auto it = std::find(g_remote_peers.begin(), g_remote_peers.end(), evt.peer);
          if (it != g_remote_peers.end()) {
            g_remote_peers.erase(it);
          }
        } else if (evt.type == ENET_EVENT_TYPE_RECEIVE) {
          ArenaReadBuffer b((const char*)evt.packet->data, evt.packet->dataLength);
          PeerCommand pc = b.Read<PeerCommand>();
          if(pc == ePC_PeerList) {
            auto numPeers = b.Read<uint32_t>();
            for (auto i = 0; i < numPeers; i++) {
              ENetAddress addr;
              addr.host = b.Read<in6_addr>();
              addr.port = b.Read<uint16_t>();

              char peerListIp[40];
              enet_address_get_host_ip(&addr, peerListIp, 40);

              Util::println("- {}:{}", peerListIp, (int)addr.port);

              ENetPeer* peerRemote = enet_host_connect(host, &addr, 2, 0);
              g_remote_peers.emplace_back(peerRemote);
            }
          } else if(pc == ePC_NewPeer) {
            ENetAddress addr;
            addr.host = b.Read<in6_addr>();
            addr.port = b.Read<uint16_t>();

            char newPeerIp[40];
            enet_address_get_host_ip(&addr, newPeerIp, 40);

            Util::println("New peer connected from {}: {}:{}", ip, newPeerIp, (int)addr.port);

            ENetPeer* peerRemote = enet_host_connect(host, &addr, 2, 0);
            g_remote_peers.emplace_back(peerRemote);
          } else if(pc == ePC_Ping) {
            Util::println("Ping from {}:{}", ip, (int)evt.peer->address.port);

            wb.Reset();
            wb.Write(ePC_Pong);
            ENetPacket* pongPacket = enet_packet_create(wb.GetBuffer(), wb.GetSize(), ENET_PACKET_FLAG_RELIABLE);
            enet_peer_send(evt.peer, 0, pongPacket);
          } else if(pc == ePC_Pong) {
            Util::println("Pong from {}:{}", ip, (int)evt.peer->address.port);
          } else {
            Util::println("Unknown command from {}:{}", ip, (int)evt.peer->address.port);
          }

          enet_packet_destroy(evt.packet);
        } else {
          Util::println("Unknown event from {}", ip);
        }
      } else {
        Util::println("... ({} remote peers)", g_remote_peers.size());
        for (auto peer : g_remote_peers) {
          wb.Reset();
          wb.Write(ePC_Ping);
          ENetPacket* packetPing = enet_packet_create(wb.GetBuffer(), wb.GetSize(), ENET_PACKET_FLAG_RELIABLE);
          enet_peer_send(peer, 0, packetPing);
        }
      }
    }

    enet_host_destroy(host);
    enet_deinitialize();
  });

  findLobbies.detach();
}

void NetplayWindow::closeEvent(QCloseEvent *event) {
  disconnect = true;
  event->accept();
  close();
}

void NetplayWindow::hideEvent(QHideEvent *event) {
  disconnect = true;
  event->accept();
  hide();
}