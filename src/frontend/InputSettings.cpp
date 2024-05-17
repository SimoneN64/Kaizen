#include <InputSettings.hpp>
#include <QVBoxLayout>
#include <QKeyEvent>
#include <log.hpp>

InputSettings::InputSettings(nlohmann::json& settings) : settings(settings), QWidget(nullptr) {
  n64_button_labels[0] = new QLabel("A");
  n64_button_labels[1] = new QLabel("B");
  n64_button_labels[2] = new QLabel("Z");
  n64_button_labels[3] = new QLabel("Start");
  n64_button_labels[4] = new QLabel("L");
  n64_button_labels[5] = new QLabel("R");
  n64_button_labels[6] = new QLabel("Dpad Up");
  n64_button_labels[7] = new QLabel("Dpad Down");
  n64_button_labels[8] = new QLabel("Dpad Left");
  n64_button_labels[9] = new QLabel("Dpad Right");
  n64_button_labels[10] = new QLabel("C Up");
  n64_button_labels[11] = new QLabel("C Down");
  n64_button_labels[12] = new QLabel("C Left");
  n64_button_labels[13] = new QLabel("C Right");
  n64_button_labels[14] = new QLabel("Analog Up");
  n64_button_labels[15] = new QLabel("Analog Down");
  n64_button_labels[16] = new QLabel("Analog Left");
  n64_button_labels[17] = new QLabel("Analog Right");
  
  auto str = JSONGetField<std::string>(settings, "input", "A");
  kb_buttons[0] = new QPushButton(str.c_str());
  str = JSONGetField<std::string>(settings, "input", "B");
  kb_buttons[1] = new QPushButton(str.c_str());
  str = JSONGetField<std::string>(settings, "input", "Z");
  kb_buttons[2] = new QPushButton(str.c_str());
  str = JSONGetField<std::string>(settings, "input", "Start");
  kb_buttons[3] = new QPushButton(str.c_str());
  str = JSONGetField<std::string>(settings, "input", "L");
  kb_buttons[4] = new QPushButton(str.c_str());
  str = JSONGetField<std::string>(settings, "input", "R");
  kb_buttons[5] = new QPushButton(str.c_str());
  str = JSONGetField<std::string>(settings, "input", "Dpad Up");
  kb_buttons[6] = new QPushButton(str.c_str());
  str = JSONGetField<std::string>(settings, "input", "Dpad Down");
  kb_buttons[7] = new QPushButton(str.c_str());
  str = JSONGetField<std::string>(settings, "input", "Dpad Left");
  kb_buttons[8] = new QPushButton(str.c_str());
  str = JSONGetField<std::string>(settings, "input", "Dpad Right");
  kb_buttons[9] = new QPushButton(str.c_str());
  str = JSONGetField<std::string>(settings, "input", "C Up");
  kb_buttons[10] = new QPushButton(str.c_str());
  str = JSONGetField<std::string>(settings, "input", "C Down");
  kb_buttons[11] = new QPushButton(str.c_str());
  str = JSONGetField<std::string>(settings, "input", "C Left");
  kb_buttons[12] = new QPushButton(str.c_str());
  str = JSONGetField<std::string>(settings, "input", "C Right");
  kb_buttons[13] = new QPushButton(str.c_str());
  str = JSONGetField<std::string>(settings, "input", "Analog Up");
  kb_buttons[14] = new QPushButton(str.c_str());
  str = JSONGetField<std::string>(settings, "input", "Analog Down");
  kb_buttons[15] = new QPushButton(str.c_str());
  str = JSONGetField<std::string>(settings, "input", "Analog Left");
  kb_buttons[16] = new QPushButton(str.c_str());
  str = JSONGetField<std::string>(settings, "input", "Analog Right");
  kb_buttons[17] = new QPushButton(str.c_str());

  for (int i = 0; i < 18; i++) {
    connect(kb_buttons[i], &QPushButton::pressed, this, [&, i]() {
      for (auto kb_button : kb_buttons) {
        kb_button->setEnabled(false);
      }
      grabKeyboard();
      grabbing = true;
      which_grabbing = i;
    });
  }

  auto AB = new QHBoxLayout;
  auto ZStart = new QHBoxLayout;
  auto LR = new QHBoxLayout;
  auto DupDdown = new QHBoxLayout;
  auto DleftDright = new QHBoxLayout;
  auto CupCdown = new QHBoxLayout;
  auto CleftCright = new QHBoxLayout;
  auto AupAdown = new QHBoxLayout;
  auto AleftAright = new QHBoxLayout;
  auto mainLayout = new QVBoxLayout;

  AB->addWidget(n64_button_labels[0]);
  AB->addWidget(kb_buttons[0]);
  AB->addWidget(n64_button_labels[1]);
  AB->addWidget(kb_buttons[1]);
  mainLayout->addLayout(AB);
  ZStart->addWidget(n64_button_labels[2]);
  ZStart->addWidget(kb_buttons[2]);
  ZStart->addWidget(n64_button_labels[3]);
  ZStart->addWidget(kb_buttons[3]);
  mainLayout->addLayout(ZStart);
  LR->addWidget(n64_button_labels[4]);
  LR->addWidget(kb_buttons[4]);
  LR->addWidget(n64_button_labels[5]);
  LR->addWidget(kb_buttons[5]);
  mainLayout->addLayout(LR);
  DupDdown->addWidget(n64_button_labels[6]);
  DupDdown->addWidget(kb_buttons[6]);
  DupDdown->addWidget(n64_button_labels[7]);
  DupDdown->addWidget(kb_buttons[7]);
  mainLayout->addLayout(DupDdown);
  DleftDright->addWidget(n64_button_labels[8]);
  DleftDright->addWidget(kb_buttons[8]);
  DleftDright->addWidget(n64_button_labels[9]);
  DleftDright->addWidget(kb_buttons[9]);
  mainLayout->addLayout(DleftDright);
  CupCdown->addWidget(n64_button_labels[10]);
  CupCdown->addWidget(kb_buttons[10]);
  CupCdown->addWidget(n64_button_labels[11]);
  CupCdown->addWidget(kb_buttons[11]);
  mainLayout->addLayout(CupCdown);
  CleftCright->addWidget(n64_button_labels[12]);
  CleftCright->addWidget(kb_buttons[12]);
  CleftCright->addWidget(n64_button_labels[13]);
  CleftCright->addWidget(kb_buttons[13]);
  mainLayout->addLayout(CleftCright);
  AupAdown->addWidget(n64_button_labels[14]);
  AupAdown->addWidget(kb_buttons[14]);
  AupAdown->addWidget(n64_button_labels[15]);
  AupAdown->addWidget(kb_buttons[15]);
  mainLayout->addLayout(AupAdown);
  AleftAright->addWidget(n64_button_labels[16]);
  AleftAright->addWidget(kb_buttons[16]);
  AleftAright->addWidget(n64_button_labels[17]);
  AleftAright->addWidget(kb_buttons[17]);
  mainLayout->addLayout(AleftAright);
  mainLayout->addStretch();
  setLayout(mainLayout);
}


void InputSettings::keyPressEvent(QKeyEvent* e) {
  if (grabbing) {
    auto k = QKeySequence(e->key()).toString();
    JSONSetField<std::string>(settings, "input", n64_button_labels[which_grabbing]->text().toStdString(), k.toStdString());
    kb_buttons[which_grabbing]->setText(k);
    grabbing = false;
    which_grabbing = -1;
    for (auto kb_button : kb_buttons) {
      kb_button->setEnabled(true);
    }
    releaseKeyboard();
    emit modified();
  }
}

std::array<Qt::Key, 18> InputSettings::GetMappedKeys() {
  std::array<Qt::Key, 18> ret{};

  for (int i = 0; i < 18; i++) {
    ret[i] = QKeySequence(kb_buttons[i]->text().toUpper())[0].key();
  }

  return ret;
}