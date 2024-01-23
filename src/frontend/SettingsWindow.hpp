#pragma once
#include <QWidget>
#include <QTabWidget>
#include <QPushButton>
#include <CPUSettings.hpp>
#include <AudioSettings.hpp>

class SettingsWindow : public QWidget {
  QPushButton* cancel = new QPushButton("Cancel");
  QPushButton* apply = new QPushButton("Apply");
  Q_OBJECT
public:
  float getVolumeL() { return float(audioSettings->volumeL->value()) / 100.f; }
  float getVolumeR() { return float(audioSettings->volumeR->value()) / 100.f; }
  SettingsWindow();
  nlohmann::json settings;
  CPUSettings* cpuSettings;
  AudioSettings* audioSettings;
};