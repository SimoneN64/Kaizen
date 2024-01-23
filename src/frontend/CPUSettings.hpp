#pragma once
#include <QWidget>
#include <QComboBox>
#include <JSONUtils.hpp>

class CPUSettings : public QWidget {
  QComboBox* cpuTypes = new QComboBox;
  Q_OBJECT
public:
  CPUSettings(nlohmann::json&);
  nlohmann::json& settings;
Q_SIGNALS:
  void modified();
};