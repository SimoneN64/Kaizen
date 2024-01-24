#pragma once
#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <JSONUtils.hpp>

class InputSettings : public QWidget {
  bool grabbing = false;
  int which_grabbing = -1;
  QPushButton* kb_buttons[18];
  QLabel* n64_button_labels[18];
  Q_OBJECT
public:
  InputSettings(nlohmann::json&);
  nlohmann::json& settings;
  void keyPressEvent(QKeyEvent*) override;
  std::array<Qt::Key, 18> GetMappedKeys();
Q_SIGNALS:
  void modified();
};