#pragma once
#include <JSONUtils.hpp>
#include <QComboBox>
#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>

class CPUSettings : public QWidget {
  std::unique_ptr<QComboBox> cpuTypes = std::make_unique<QComboBox>();
  std::unique_ptr<QLabel> label = std::make_unique<QLabel>("CPU type:");
  std::unique_ptr<QVBoxLayout> mainLayout = std::make_unique<QVBoxLayout>();
  Q_OBJECT
public:
  CPUSettings(nlohmann::json &);
  nlohmann::json &settings;
Q_SIGNALS:
  void modified();
};
