#pragma once
#include <QWidget>
#include <QTabWidget>
#include <QPushButton>
#include <CPUSettings.hpp>

class SettingsWindow : public QWidget {
  QPushButton* cancel = new QPushButton("Cancel");
  QPushButton* apply = new QPushButton("Apply");
  Q_OBJECT
public:
  SettingsWindow();
  nlohmann::json settings;
  CPUSettings* cpuSettings;
};