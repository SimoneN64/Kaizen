#pragma once
#include <QWidget>
#include <QTabWidget>
#include <QPushButton>
#include <arena.hpp>

typedef struct _ENetHost ENetHost;
typedef struct _ENetPeer ENetPeer;

class NetplayWindow : public QWidget {
  ENetPeer* peer{};
  ENetHost* host{};
  ArenaBuffer wb;
  Q_OBJECT
public:
  NetplayWindow();
};