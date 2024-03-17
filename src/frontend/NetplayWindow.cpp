#define ENET_IMPLEMENTATION
#include <enet.h>
#include <NetplayWindow.hpp>
#include <QCloseEvent>
#include <QHideEvent>
#include <thread>
#include <log.hpp>
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

template <typename ...Args>
void SendPacket(ArenaBuffer& wb, ENetPeer* dest, Args... args) {
  wb.Reset();
  wb.Write(args...);
  ENetPacket* packet = enet_packet_create(wb.GetBuffer(), wb.GetSize(), ENET_PACKET_FLAG_RELIABLE);
  enet_peer_send(dest, 0, packet);
}

template <typename ...Args>
void SendPacket(ArenaBuffer& wb, const std::vector<ENetPeer*>& dests, Args... args) {
  wb.Reset();
  wb.Write(args...);
  ENetPacket* packet = enet_packet_create(wb.GetBuffer(), wb.GetSize(), ENET_PACKET_FLAG_RELIABLE);
  for (auto dest : dests) {
    enet_peer_send(dest, 0, packet);
  }
}

template <typename T>
void SendPacket(ArenaBuffer& wb, const std::vector<ENetPeer*>& dests, std::vector<T> data) {
  wb.Reset();
  wb.Write(data);
  ENetPacket *packet = enet_packet_create(wb.GetBuffer(), wb.GetSize(), ENET_PACKET_FLAG_RELIABLE);
  for(auto dest : dests) {
    enet_peer_send(dest, 0, packet);
  }
}

void NetplayWindow::MainNetLoop() {
  if(!toConnect) return;

  std::string passcode{};
  ENetEvent evt;
  while (enet_host_service(host, &evt, 0) > 0) {
    switch (evt.type) {
      case ENET_EVENT_TYPE_CONNECT:
        if(wantsToCreateLobby)
          SendPacket(wb, evt.peer, eSCMD_CreateLobby);
        else
          SendPacket(wb, evt.peer, eSCMD_JoinLobby);
        break;
      case ENET_EVENT_TYPE_RECEIVE: {
        ArenaReadBuffer b{ evt.packet->data, evt.packet->dataLength };
        auto command = b.Read<uint8_t>();
        switch (command) {
          case eCCMD_LobbyIsFull:
            QMessageBox::critical(
            this, "eCCMD_LobbyIsFull",
            "Please ask for help in my discord server");
            break;
          case eCCMD_PasscodeIncorrect:
            QMessageBox::critical(
            this, "eCCMD_PasscodeIncorrect",
            "Please ask for help in my discord server");
            break;
          case eCCMD_MaxLobbiesReached:
            QMessageBox::critical(
            this, tr("Sorry :("),
            tr("My server is not that powerful,\n"
               "so more than 16 rooms can't be created\n"));
            break;
          case eCCMD_LobbyChanged: break;
          case eCCMD_Passcode: {
            passcode = b.Read<std::string>();
            passcodeLabel->setText(passcode.c_str());
          }
          default: break;
        }
      } break;
      case ENET_EVENT_TYPE_DISCONNECT:
      case ENET_EVENT_TYPE_DISCONNECT_TIMEOUT:
      default: break;
    }
  }
}

void NetplayWindow::keyPressEvent(QKeyEvent *event) {
  grabKeyboard();
  event->accept();
}

void NetplayWindow::keyReleaseEvent(QKeyEvent *event) {
  releaseKeyboard();
  event->accept();
}

NetplayWindow::NetplayWindow() {
  if (objectName().isEmpty())
    setObjectName("NetplayWindow");

  resize(500, 400);
  setWindowTitle("Netplay");

  if (enet_initialize() != 0) {
    Util::panic("Could not initialize enet!");
  }

  ENetAddress address = {};
  address.host = ENET_HOST_ANY;
  address.port = 7788;

  host = enet_host_create(nullptr, 4, 2, 0, 0);
  if (!host) {
    Util::panic("Could not create host");
  }

  enet_address_set_host(&address, "127.0.0.1");
  address.port = 7788;
  peer = enet_host_connect(host, &address, 2, 0);
  if (!peer) {
    Util::panic("Could not connect");
  }

  netTimer = new QTimer(this);
  connect(netTimer, SIGNAL(timeout()), this, SLOT(MainNetLoop()));
  netTimer->start(16);

  auto tabs = new QTabWidget;
  auto createRoomWidget = new QTabBar;
  auto createRoomLayout = new QVBoxLayout;
  auto createRoomLayoutH = new QHBoxLayout;
  auto createButton = new QPushButton("Create");
  passcodeLabel = new QLabel("");
  createRoomLayoutH->addWidget(createButton);
  createRoomLayoutH->addWidget(new QLabel("The passcode:"));
  createRoomLayoutH->addWidget(passcodeLabel);
  createRoomLayout->addLayout(createRoomLayoutH);
  createRoomLayout->addStretch();
  createRoomWidget->setLayout(createRoomLayout);
  auto joinRoomWidget = new QTabBar;
  passcodeInput = new QPlainTextEdit;
  passcodeInput->setPlaceholderText("kj3l87");
  passcodeInput->setFixedSize(60, 30);
  passcodeInput->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  passcodeInput->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  auto joinButton = new QPushButton("Join");
  auto joinRoomLayout = new QVBoxLayout;
  auto joinRoomLayoutH = new QHBoxLayout;
  joinRoomLayoutH->addWidget(passcodeInput);
  joinRoomLayoutH->addWidget(joinButton);
  joinRoomLayout->addLayout(joinRoomLayoutH);
  joinRoomLayout->addStretch();
  joinRoomWidget->setLayout(joinRoomLayout);

  connect(createButton, &QPushButton::pressed, this, [&]() {
    wantsToCreateLobby = true;
    toConnect = true;
  });

  connect(joinButton, &QPushButton::pressed, this, [&]() {
    wantsToCreateLobby = false;
    toConnect = true;
  });

  tabs->addTab(createRoomWidget, "Create a room");
  tabs->addTab(joinRoomWidget, "Join a room");
  QVBoxLayout* mainLayout = new QVBoxLayout;
  mainLayout->addWidget(tabs);
  setLayout(mainLayout);
}