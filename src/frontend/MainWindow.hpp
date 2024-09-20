#pragma once
#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <RenderWidget.hpp>
#include <ImGuiWidget.hpp>

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  MainWindow() noexcept;

  QAction *actionAbout;
  QAction *actionOpen;
  QAction *actionExit;
  QAction *actionPause;
  QAction *actionReset;
  QAction *actionStop;
  QAction *actionSettings;
  QWidget *centralwidget;
  QVBoxLayout *verticalLayout;
  QHBoxLayout *horizontalLayout;
  RenderWidget *vulkanWidget;
  ImGuiWidget *debugger;
  QMenuBar *menubar;
  QMenu *menuFile;
  QMenu *menuEmulation;
  QMenu *menuAbout;
  QStatusBar *statusbar;

private:
  void Retranslate();
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
