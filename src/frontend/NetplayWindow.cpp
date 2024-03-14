#define ENET_IMPLEMENTATION
#include <enet.h>
#include <NetplayWindow.hpp>
#include <QCloseEvent>
#include <QHideEvent>
#include <thread>
#include <log.hpp>
#include <arena.hpp>
#include <QBoxLayout>
#include <QTabBar>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QLabel>

enum ServerSideCommand : uint8_t {
  eSCMD_None,
  eSCMD_JoinLobby,
  eSCMD_CreateLobby,
};

enum ClientSideCommand : uint8_t {
  eCCMD_None,
  eCCMD_LobbyIsFull,
  eCCMD_PasscodeIncorrect,
  eCCMD_MaxLobbiesReached,
  eCCMD_LobbyChanged,
  eCCMD_Passcode,
};

template <typename ...Args>
void SendPacket(ArenaBuffer& wb, ENetPeer* dest, Args... args) {
  wb.Reset();
  wb.Write(args...);
  ENetPacket* packet = enet_packet_create(wb.GetBuffer(), wb.GetSize(), ENET_PACKET_FLAG_RELIABLE);
  enet_peer_send(dest, 0, packet);
}

template <typename ...Args>
void SendPacket(ArenaBuffer& wb, std::vector<ENetPeer*> dests, Args... args) {
  wb.Reset();
  wb.Write(args...);
  ENetPacket* packet = enet_packet_create(wb.GetBuffer(), wb.GetSize(), ENET_PACKET_FLAG_RELIABLE);
  for (auto dest : dests) {
    enet_peer_send(dest, 0, packet);
  }
}

NetplayWindow::NetplayWindow() {
  if (objectName().isEmpty())
    setObjectName("Netplay");

  resize(500, 400);
  setWindowTitle("Netplay");

  if (enet_initialize() != 0) {
    Util::panic("Could not initialize enet!");
  }

  ENetAddress address = {};
  address.host = ENET_HOST_ANY;
  address.port = 7788;

  ENetHost* host = enet_host_create(nullptr, 4, 2, 0, 0);
  if (!host) {
    Util::panic("Could not create host");
  }

  enet_address_set_host(&address, "gadolinium.dev");
  address.port = 7788;
  ENetPeer* peer = enet_host_connect(host, &address, 2, 0);
  if (!peer) {
    Util::panic("Could not connect");
  }

  std::string passcode{};
  ArenaBuffer wb;

  auto tabs = new QTabWidget;
  auto createRoomWidget = new QTabBar;
  auto createRoomLayout = new QHBoxLayout;
  auto passcodeLabel = new QLabel("");
  createRoomLayout->addWidget(passcodeLabel);
  createRoomWidget->setLayout(createRoomLayout);
  auto joinRoomWidget = new QTabBar;
  auto passcodeInput = new QPlainTextEdit;
  passcodeInput->setPlaceholderText("Passcode (eg: AbcD31)");
  auto joinButton = new QPushButton("Join");
  auto joinRoomLayout = new QHBoxLayout;
  joinRoomLayout->addWidget(passcodeInput);
  joinRoomLayout->addWidget(joinButton);
  joinRoomWidget->setLayout(joinRoomLayout);

  connect(createRoomWidget, &QTabBar::hasFocus, this, [&]() {
    bool loop = true;
    while (loop) {
      ENetEvent evt;
      while (enet_host_service(host, &evt, 0) > 0) {
        switch (evt.type) {
        case ENET_EVENT_TYPE_CONNECT: break;
        case ENET_EVENT_TYPE_RECEIVE: {
          ArenaReadBuffer b{ (char*)evt.packet->data, evt.packet->dataLength };
          auto command = b.Read<ClientSideCommand>();
          switch (command) {
            case eCCMD_LobbyIsFull:
              QMessageBox::critical(
                this, tr("What?!"),
                tr("You just tried to create a room...\n"
                  "so how can you possibily encounter a full room?\n"));
              loop = false;
              break;
            case eCCMD_PasscodeIncorrect:
              QMessageBox::critical(
                this, tr("What?!"),
                tr("You just tried to create a room...\n"
                  "so how can you possibily have entered an incorrect passcode?\n"));
              loop = false;
              break;
            case eCCMD_MaxLobbiesReached:
              QMessageBox::critical(
                this, tr("Sorry :("),
                tr("My server is not that powerful,\n"
                  "so more than 16 rooms can't be created\n"));
              loop = false;
              break;
            case eCCMD_LobbyChanged: break;
            case eCCMD_Passcode:
              passcode = b.Read();
              passcodeLabel->setText(passcode.c_str());
              loop = false;
            default: break;
          }
        } break;
        case ENET_EVENT_TYPE_DISCONNECT:
        case ENET_EVENT_TYPE_DISCONNECT_TIMEOUT:
          loop = false;
        default: break;
        }
      }
    }
  });

  connect(joinButton, &QPushButton::clicked, this, [&]() {
    bool loop = true;
    while (loop) {
      ENetEvent evt;
      while (enet_host_service(host, &evt, 0) > 0) {
        switch (evt.type) {
        case ENET_EVENT_TYPE_CONNECT: break;
        case ENET_EVENT_TYPE_RECEIVE: {
          ArenaReadBuffer b{ (char*)evt.packet->data, evt.packet->dataLength };
          auto command = b.Read<ClientSideCommand>();
          switch (command) {
          case eCCMD_LobbyIsFull:
            QMessageBox::critical(
              this, tr("Oops"),
              tr("This lobby is full"));
            loop = false;
            break;
          case eCCMD_PasscodeIncorrect:
            QMessageBox::critical(
              this, tr("Oops"),
              tr("The passcode you entered is either incorrect or doesn't exist"));
            loop = false;
            break;
          case eCCMD_MaxLobbiesReached:
            QMessageBox::critical(
              this, tr("What?!"),
              tr("You are not creating a lobby, so why did you receive the error that max lobbies are reached?"));
            loop = false;
            break;
          case eCCMD_LobbyChanged: break;
          case eCCMD_Passcode:
            QMessageBox::critical(
              this, tr("What?!"),
              tr("You are not creating a lobby, so why did you receive a passcode from the server?"));
            loop = false;
          default: break;
          }
        } break;
        case ENET_EVENT_TYPE_DISCONNECT:
        case ENET_EVENT_TYPE_DISCONNECT_TIMEOUT:
          loop = false;
        default: break;
        }
      }
    }
  });
  tabs->addTab(createRoomWidget, "Create a room");
  tabs->addTab(joinRoomWidget, "Join a room");
  QVBoxLayout* mainLayout = new QVBoxLayout;
  mainLayout->addWidget(tabs);
  setLayout(mainLayout);
}