#pragma once
#include <QApplication>
#include <QMainWindow>
#include <RenderWidget.hpp>
#include "ui_mainwindow.h"

class MainWindowController : public QMainWindow {
  Q_OBJECT

public:
  MainWindowController() noexcept;

  Ui::MainWindow view;

private:
  void ConnectSignalsToSlots() noexcept;

  bool textPauseToggle = false;

Q_SIGNALS:
  void OpenSettings();
  void OpenROM(const QString &rom_file);
  void Exit();
  void Reset();
  void Stop();
  void Pause();
};
