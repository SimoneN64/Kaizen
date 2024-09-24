#include <InputSettings.hpp>
#include <log.hpp>

InputSettings::InputSettings(nlohmann::json &settings) : settings(settings), QWidget(nullptr) {
  n64_button_labels[0]  = std::make_unique<QLabel>("A");
  n64_button_labels[1]  = std::make_unique<QLabel>("B");
  n64_button_labels[2]  = std::make_unique<QLabel>("Z");
  n64_button_labels[3]  = std::make_unique<QLabel>("Start");
  n64_button_labels[4]  = std::make_unique<QLabel>("L");
  n64_button_labels[5]  = std::make_unique<QLabel>("R");
  n64_button_labels[6]  = std::make_unique<QLabel>("Dpad Up");
  n64_button_labels[7]  = std::make_unique<QLabel>("Dpad Down");
  n64_button_labels[8]  = std::make_unique<QLabel>("Dpad Left");
  n64_button_labels[9]  = std::make_unique<QLabel>("Dpad Right");
  n64_button_labels[10] = std::make_unique<QLabel>("C Up");
  n64_button_labels[11] = std::make_unique<QLabel>("C Down");
  n64_button_labels[12] = std::make_unique<QLabel>("C Left");
  n64_button_labels[13] = std::make_unique<QLabel>("C Right");
  n64_button_labels[14] = std::make_unique<QLabel>("Analog Up");
  n64_button_labels[15] = std::make_unique<QLabel>("Analog Down");
  n64_button_labels[16] = std::make_unique<QLabel>("Analog Left");
  n64_button_labels[17] = std::make_unique<QLabel>("Analog Right");

  auto str = JSONGetField<std::string>(settings, "input", "A");
  kb_buttons[0] = std::make_unique<QPushButton>(str.c_str());
  str = JSONGetField<std::string>(settings, "input", "B");
  kb_buttons[1] = std::make_unique<QPushButton>(str.c_str());
  str = JSONGetField<std::string>(settings, "input", "Z");
  kb_buttons[2] = std::make_unique<QPushButton>(str.c_str());
  str = JSONGetField<std::string>(settings, "input", "Start");
  kb_buttons[3] = std::make_unique<QPushButton>(str.c_str());
  str = JSONGetField<std::string>(settings, "input", "L");
  kb_buttons[4] = std::make_unique<QPushButton>(str.c_str());
  str = JSONGetField<std::string>(settings, "input", "R");
  kb_buttons[5] = std::make_unique<QPushButton>(str.c_str());
  str = JSONGetField<std::string>(settings, "input", "Dpad Up");
  kb_buttons[6] = std::make_unique<QPushButton>(str.c_str());
  str = JSONGetField<std::string>(settings, "input", "Dpad Down");
  kb_buttons[7] = std::make_unique<QPushButton>(str.c_str());
  str = JSONGetField<std::string>(settings, "input", "Dpad Left");
  kb_buttons[8] = std::make_unique<QPushButton>(str.c_str());
  str = JSONGetField<std::string>(settings, "input", "Dpad Right");
  kb_buttons[9] = std::make_unique<QPushButton>(str.c_str());
  str = JSONGetField<std::string>(settings, "input", "C Up");
  kb_buttons[10] = std::make_unique<QPushButton>(str.c_str());
  str = JSONGetField<std::string>(settings, "input", "C Down");
  kb_buttons[11] = std::make_unique<QPushButton>(str.c_str());
  str = JSONGetField<std::string>(settings, "input", "C Left");
  kb_buttons[12] = std::make_unique<QPushButton>(str.c_str());
  str = JSONGetField<std::string>(settings, "input", "C Right");
  kb_buttons[13] = std::make_unique<QPushButton>(str.c_str());
  str = JSONGetField<std::string>(settings, "input", "Analog Up");
  kb_buttons[14] = std::make_unique<QPushButton>(str.c_str());
  str = JSONGetField<std::string>(settings, "input", "Analog Down");
  kb_buttons[15] = std::make_unique<QPushButton>(str.c_str());
  str = JSONGetField<std::string>(settings, "input", "Analog Left");
  kb_buttons[16] = std::make_unique<QPushButton>(str.c_str());
  str = JSONGetField<std::string>(settings, "input", "Analog Right");
  kb_buttons[17] = std::make_unique<QPushButton>(str.c_str());

  for (int i = 0; i < 18; i++) {
    connect(kb_buttons[i].get(), &QPushButton::pressed, this, [&, i]() {
      for (auto& kb_button : kb_buttons) {
        kb_button->setEnabled(false);
      }
      grabKeyboard();
      grabbing = true;
      which_grabbing = i;
    });
  }

  AB->addWidget(n64_button_labels[0].get());
  AB->addWidget(kb_buttons[0].get());
  AB->addWidget(n64_button_labels[1].get());
  AB->addWidget(kb_buttons[1].get());
  mainLayout->addLayout(AB.get());
  ZStart->addWidget(n64_button_labels[2].get());
  ZStart->addWidget(kb_buttons[2].get());
  ZStart->addWidget(n64_button_labels[3].get());
  ZStart->addWidget(kb_buttons[3].get());
  mainLayout->addLayout(ZStart.get());
  LR->addWidget(n64_button_labels[4].get());
  LR->addWidget(kb_buttons[4].get());
  LR->addWidget(n64_button_labels[5].get());
  LR->addWidget(kb_buttons[5].get());
  mainLayout->addLayout(LR.get());
  DupDdown->addWidget(n64_button_labels[6].get());
  DupDdown->addWidget(kb_buttons[6].get());
  DupDdown->addWidget(n64_button_labels[7].get());
  DupDdown->addWidget(kb_buttons[7].get());
  mainLayout->addLayout(DupDdown.get());
  DleftDright->addWidget(n64_button_labels[8].get());
  DleftDright->addWidget(kb_buttons[8].get());
  DleftDright->addWidget(n64_button_labels[9].get());
  DleftDright->addWidget(kb_buttons[9].get());
  mainLayout->addLayout(DleftDright.get());
  CupCdown->addWidget(n64_button_labels[10].get());
  CupCdown->addWidget(kb_buttons[10].get());
  CupCdown->addWidget(n64_button_labels[11].get());
  CupCdown->addWidget(kb_buttons[11].get());
  mainLayout->addLayout(CupCdown.get());
  CleftCright->addWidget(n64_button_labels[12].get());
  CleftCright->addWidget(kb_buttons[12].get());
  CleftCright->addWidget(n64_button_labels[13].get());
  CleftCright->addWidget(kb_buttons[13].get());
  mainLayout->addLayout(CleftCright.get());
  AupAdown->addWidget(n64_button_labels[14].get());
  AupAdown->addWidget(kb_buttons[14].get());
  AupAdown->addWidget(n64_button_labels[15].get());
  AupAdown->addWidget(kb_buttons[15].get());
  mainLayout->addLayout(AupAdown.get());
  AleftAright->addWidget(n64_button_labels[16].get());
  AleftAright->addWidget(kb_buttons[16].get());
  AleftAright->addWidget(n64_button_labels[17].get());
  AleftAright->addWidget(kb_buttons[17].get());
  mainLayout->addLayout(AleftAright.get());
  mainLayout->addStretch();
  setLayout(mainLayout.get());
}


void InputSettings::keyPressEvent(QKeyEvent *e) {
  if (grabbing) {
    auto k = QKeySequence(e->key()).toString();
    JSONSetField<std::string>(settings, "input", n64_button_labels[which_grabbing]->text().toStdString(),
                              k.toStdString());
    kb_buttons[which_grabbing]->setText(k);
    grabbing = false;
    which_grabbing = -1;
    for (auto& kb_button : kb_buttons) {
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
