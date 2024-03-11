#pragma once
#include <QWidget>
#include <QTabWidget>
#include <QPushButton>

class NetplayWindow : public QWidget {
  std::vector<QPushButton*> lobbies{};
  bool disconnect = false;
  Q_OBJECT
public:
  void showEvent(QShowEvent *event) override;
  void closeEvent(QCloseEvent *event) override;
  void hideEvent(QHideEvent *event) override;
};