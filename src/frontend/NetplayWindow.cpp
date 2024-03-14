#define ENET_IMPLEMENTATION
#include <enet.h>
#include <NetplayWindow.hpp>
#include <QCloseEvent>
#include <QHideEvent>
#include <thread>
#include <log.hpp>
#include <arena.hpp>
#include <QBoxLayout>

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

  auto tabs = new QTabWidget;
  auto createRoomWidget = new QWidget;
  auto joinRoomWidget = new QWidget;
  tabs->addTab(createRoomWidget, "Create a room");
  tabs->addTab(joinRoomWidget, "Join a room");
  QVBoxLayout* mainLayout = new QVBoxLayout;
  mainLayout->addWidget(tabs);
  setLayout(mainLayout);
}