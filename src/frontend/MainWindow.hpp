#pragma once
#include "ui_mainwindow.h"
#include <RenderWidget.hpp>
#include <QMainWindow>
#include <QApplication>

class MainWindowController : public QMainWindow
{
  Q_OBJECT

public:
  MainWindowController() noexcept;

  Ui::MainWindow view;
private:
  void ConnectSignalsToSlots() noexcept;

  bool textPauseToggle = false;

Q_SIGNALS:
  void OpenNetplay();
  void OpenSettings();
  void OpenROM(const QString& rom_file);
  void Exit();
  void Reset();
  void Stop();
  void Pause();
};