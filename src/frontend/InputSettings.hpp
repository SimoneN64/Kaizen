#pragma once
#include <JSONUtils.hpp>
#include <QLabel>
#include <QPushButton>
#include <QWidget>
#include <QKeyEvent>
#include <QVBoxLayout>

class InputSettings : public QWidget {
  bool grabbing = false;
  int which_grabbing = -1;

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
  std::array<std::unique_ptr<QPushButton>, 18> kb_buttons;
  std::array<std::unique_ptr<QLabel>, 18> n64_button_labels;
  Q_OBJECT
public:
  InputSettings(nlohmann::json &);
  nlohmann::json &settings;
  void keyPressEvent(QKeyEvent *) override;
  std::array<Qt::Key, 18> GetMappedKeys();
Q_SIGNALS:
  void modified();
};
