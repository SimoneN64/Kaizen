#pragma once
#include <QWidget>
#include <QTabWidget>
#include <QPushButton>
#include <arena.hpp>
#include <QLabel>
#include <QTimer>
#include <QPlainTextEdit>

typedef struct _ENetHost ENetHost;
typedef struct _ENetPeer ENetPeer;

class NetplayWindow : public QWidget {
  ENetPeer* peer{};
  ENetHost* host{};
  QLabel* passcodeLabel;
  QPlainTextEdit* passcodeInput;
  QTimer* netTimer;
  bool wantsToCreateLobby = false;
  bool toConnect = false;
  ArenaBuffer wb;
  Q_OBJECT
public:
  NetplayWindow();
  void keyPressEvent(QKeyEvent *event) override;
  void keyReleaseEvent(QKeyEvent *event) override;
public slots:
  void MainNetLoop();
};