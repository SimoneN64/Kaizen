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

  auto tabs = new QTabWidget;
  auto createRoomWidget = new QTabBar;
  auto joinRoomWidget = new QTabBar;
  connect(createRoomWidget, &QTabBar::tabBarClicked, this, [&]() {
    while (true) {
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
              goto STOP;
            case eCCMD_PasscodeIncorrect:
              QMessageBox::critical(
                this, tr("What?!"),
                tr("You just tried to create a room...\n"
                  "so how can you possibily have entered an incorrect passcode?\n"));
              goto STOP;
            case eCCMD_MaxLobbiesReached:
              QMessageBox::critical(
                this, tr("Sorry :("),
                tr("My server is not that powerful,\n"
                  "so more than 16 rooms can't be created\n"));
              goto STOP;
            case eCCMD_LobbyChanged: break;
            case eCCMD_Passcode:
              passcode = b.Read();
              goto STOP;
            default: break;
          }
        } break;
        case ENET_EVENT_TYPE_DISCONNECT:
        case ENET_EVENT_TYPE_DISCONNECT_TIMEOUT:
          goto STOP;
        default: break;
        }
      }
    }
    STOP:
  });

  connect(joinRoomWidget, &QTabBar::tabBarClicked, this, [&]() {
    while (true) {
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
            goto STOP;
          case eCCMD_PasscodeIncorrect:
            QMessageBox::critical(
              this, tr("Oops"),
              tr("The passcode you entered is either incorrect or doesn't exist"));
            goto STOP;
          case eCCMD_MaxLobbiesReached:
            QMessageBox::critical(
              this, tr("What?!"),
              tr("You are not creating a lobby, so why did you receive the error that max lobbies are reached?"));
            goto STOP;
          case eCCMD_LobbyChanged: break;
          case eCCMD_Passcode:
            QMessageBox::critical(
              this, tr("What?!"),
              tr("You are not creating a lobby, so why did you receive a passcode from the server?"));
            goto STOP;
          default: break;
          }
        } break;
        case ENET_EVENT_TYPE_DISCONNECT:
        case ENET_EVENT_TYPE_DISCONNECT_TIMEOUT:
          goto STOP;
        default: break;
        }
      }
    }
    STOP:
  });
  tabs->addTab(createRoomWidget, "Create a room");
  tabs->addTab(joinRoomWidget, "Join a room");
  QVBoxLayout* mainLayout = new QVBoxLayout;
  mainLayout->addWidget(tabs);
  setLayout(mainLayout);
}