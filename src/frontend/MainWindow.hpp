#pragma once
#include <QMainWindow>
#include "ui_mainwindow.h"

class MainWindowController : public QMainWindow
{
  Q_OBJECT

public:
  MainWindowController() noexcept;

private:
  void ConnectSignalsToSlots() noexcept;

  Ui::MainWindow view;
  bool textPauseToggle = false;

signals:
  void OpenROM(const QString& rom_file);
  void Exit();
  void Reset();
  void Stop();
  void Pause();
};