#pragma once
#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QKeyEvent>
#include <QMessageBox>
#include <QSlider>
#include <QMenu>
#include <QMenuBar>
#include <QStatusBar>
#include <RenderWidget.hpp>
#include <Debugger.hpp>

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  MainWindow(const std::shared_ptr<n64::Core> &) noexcept;

  std::unique_ptr<QAction> actionOpenDebuggerWindow{};
  std::unique_ptr<QAction> actionAbout{};
  std::unique_ptr<QAction> actionOpen{};
  std::unique_ptr<QAction> actionExit{};
  std::unique_ptr<QAction> actionPause{};
  std::unique_ptr<QAction> actionReset{};
  std::unique_ptr<QAction> actionStop{};
  std::unique_ptr<QAction> actionSettings{};
  std::unique_ptr<QWidget> centralwidget{};
  std::unique_ptr<QVBoxLayout> verticalLayout{};
  std::unique_ptr<RenderWidget> vulkanWidget{};
  std::unique_ptr<QMenuBar> menubar{};
  std::unique_ptr<QMenu> menuFile{};
  std::unique_ptr<QMenu> menuEmulation{};
  std::unique_ptr<QMenu> menuTools{};
  std::unique_ptr<QMenu> menuAbout{};
  std::unique_ptr<QStatusBar> statusbar{};

private:
  void Retranslate();
  void ConnectSignalsToSlots() noexcept;

  bool textPauseToggle = false;

Q_SIGNALS:
  void OpenDebugger();
  void OpenSettings();
  void OpenROM(const QString &rom_file);
  void Exit();
  void Reset();
  void Stop();
  void Pause();
};
