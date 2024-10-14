#pragma once
#include <JSONUtils.hpp>
#include <QLabel>
#include <QPushButton>
#include <QWidget>
#include <QKeyEvent>
#include <QVBoxLayout>
#include <QComboBox>
#include <QTimer>

class InputSettings final : public QWidget {
  bool grabbing = false;
  int whichGrabbing = -1;

  void QueryDevices() noexcept;
  void PollGamepad() noexcept;

  std::unordered_map<u32, std::string> gamepadIndexes{};

  std::unique_ptr<QHBoxLayout> AB = std::make_unique<QHBoxLayout>();
  std::unique_ptr<QHBoxLayout> ZStart = std::make_unique<QHBoxLayout>();
  std::unique_ptr<QHBoxLayout> LR = std::make_unique<QHBoxLayout>();
  std::unique_ptr<QHBoxLayout> DupDdown = std::make_unique<QHBoxLayout>();
  std::unique_ptr<QHBoxLayout> DleftDright = std::make_unique<QHBoxLayout>();
  std::unique_ptr<QHBoxLayout> CupCdown = std::make_unique<QHBoxLayout>();
  std::unique_ptr<QHBoxLayout> CleftCright = std::make_unique<QHBoxLayout>();
  std::unique_ptr<QHBoxLayout> AupAdown = std::make_unique<QHBoxLayout>();
  std::unique_ptr<QHBoxLayout> AleftAright = std::make_unique<QHBoxLayout>();
  std::unique_ptr<QVBoxLayout> mainLayout = std::make_unique<QVBoxLayout>();
  std::array<std::unique_ptr<QPushButton>, 18> kbButtons;
  std::array<std::unique_ptr<QLabel>, 18> buttonLabels;
  std::unique_ptr<QHBoxLayout> deviceComboBoxLayout = std::make_unique<QHBoxLayout>();
  QTimer refresh, pollGamepad;
  std::unique_ptr<QLabel> devicesLabel = std::make_unique<QLabel>("Device:");
  std::unique_ptr<QComboBox> devices = std::make_unique<QComboBox>();
  Q_OBJECT
public:
  bool selectedDeviceIsNotKeyboard = false;
  explicit InputSettings(nlohmann::json &);
  nlohmann::json &settings;
  void keyPressEvent(QKeyEvent *) override;
  std::array<Qt::Key, 18> GetMappedKeys() const;
Q_SIGNALS:
  void modified();
};
