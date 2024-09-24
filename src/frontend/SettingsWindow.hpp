#pragma once
#include <AudioSettings.hpp>
#include <CPUSettings.hpp>
#include <InputSettings.hpp>
#include <QFileIconProvider>
#include <QPushButton>
#include <QTabWidget>
#include <QWidget>
#include <QButtonGroup>
#include <QFileDialog>
#include <QGroupBox>
#include <QVBoxLayout>

class SettingsWindow : public QWidget {
  std::unique_ptr<QPushButton> cancel = std::make_unique<QPushButton>("Cancel");
  std::unique_ptr<QPushButton> apply = std::make_unique<QPushButton>("Apply");
  std::unique_ptr<QFileIconProvider> iconProv = std::make_unique<QFileIconProvider>();
  std::unique_ptr<QPushButton> folderBtn = std::make_unique<QPushButton>(iconProv->icon(QFileIconProvider::Folder), "");
  std::unique_ptr<QLabel> folderLabelPrefix = std::make_unique<QLabel>("Save files' path: ");
  std::unique_ptr<QLabel> folderLabel;
  std::unique_ptr<QHBoxLayout> generalLayout = std::make_unique<QHBoxLayout>();
  std::unique_ptr<QVBoxLayout> generalLayoutV = std::make_unique<QVBoxLayout>();
  std::unique_ptr<QTabWidget> tabs = std::make_unique<QTabWidget>();
  std::unique_ptr<QVBoxLayout> mainLayout = std::make_unique<QVBoxLayout>();
  std::unique_ptr<QHBoxLayout> buttonsLayout = std::make_unique<QHBoxLayout>();
  Q_OBJECT
public:
  float getVolumeL() { return float(audioSettings->volumeL->value()) / 100.f; }
  float getVolumeR() { return float(audioSettings->volumeR->value()) / 100.f; }
  std::array<Qt::Key, 18> keyMap{};
  SettingsWindow();
  nlohmann::json settings;
  std::unique_ptr<CPUSettings> cpuSettings{};
  std::unique_ptr<AudioSettings> audioSettings{};
  std::unique_ptr<InputSettings> inputSettings{};
  std::unique_ptr<QWidget> generalSettings{};
Q_SIGNALS:
  void regrabKeyboard();
};
