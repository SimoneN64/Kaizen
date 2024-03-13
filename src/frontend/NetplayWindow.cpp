#define ENET_IMPLEMENTATION
#include <enet.h>
#include <NetplayWindow.hpp>
#include <QCloseEvent>
#include <QHideEvent>
#include <thread>
#include <log.hpp>
#include <arena.hpp>

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

