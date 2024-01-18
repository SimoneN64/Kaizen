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

private:
  void ConnectSignalsToSlots() noexcept;

  Ui::MainWindow view;
  RenderWidget *vulkanWidget;
  bool textPauseToggle = false;

Q_SIGNALS:
  void OpenROM(const QString& rom_file);
  void Exit();
  void Reset();
  void Stop();
  void Pause();
};