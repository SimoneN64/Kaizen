#pragma once
#include <QWidget>
#include <QTabWidget>
#include <QDialogButtonBox>
#include <CPUSettings.hpp>
#include <AudioSettings.hpp>
#include <InputSettings.hpp>

class SettingsWindow : public QWidget {
  QDialogButtonBox* buttonBox;
  Q_OBJECT
public:
  float getVolumeL() { return float(audioSettings->volumeL->value()) / 100.f; }
  float getVolumeR() { return float(audioSettings->volumeR->value()) / 100.f; }
  std::array<Qt::Key, 18> keyMap{};
  SettingsWindow();
  nlohmann::json settings;
  CPUSettings* cpuSettings;
  AudioSettings* audioSettings;
  InputSettings* inputSettings;
Q_SIGNALS:
  void regrabKeyboard();
};