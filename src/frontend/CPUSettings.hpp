#pragma once
#include <JSONUtils.hpp>
#include <QComboBox>
#include <QWidget>

class CPUSettings : public QWidget {
  QComboBox *cpuTypes = new QComboBox;
  Q_OBJECT
public:
  CPUSettings(nlohmann::json &);
  nlohmann::json &settings;
Q_SIGNALS:
  void modified();
};
