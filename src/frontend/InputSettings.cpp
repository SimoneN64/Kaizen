#include <InputSettings.hpp>
#include <log.hpp>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_init.h>

InputSettings::InputSettings(nlohmann::json &settings) : QWidget(nullptr), settings(settings) {
  buttonLabels[0] = std::make_unique<QLabel>("A");
  buttonLabels[1] = std::make_unique<QLabel>("B");
  buttonLabels[2] = std::make_unique<QLabel>("Z");
  buttonLabels[3] = std::make_unique<QLabel>("Start");
  buttonLabels[4] = std::make_unique<QLabel>("L");
  buttonLabels[5] = std::make_unique<QLabel>("R");
  buttonLabels[6] = std::make_unique<QLabel>("Dpad Up");
  buttonLabels[7] = std::make_unique<QLabel>("Dpad Down");
  buttonLabels[8] = std::make_unique<QLabel>("Dpad Left");
  buttonLabels[9] = std::make_unique<QLabel>("Dpad Right");
  buttonLabels[10] = std::make_unique<QLabel>("C Up");
  buttonLabels[11] = std::make_unique<QLabel>("C Down");
  buttonLabels[12] = std::make_unique<QLabel>("C Left");
  buttonLabels[13] = std::make_unique<QLabel>("C Right");
  buttonLabels[14] = std::make_unique<QLabel>("Analog Up");
  buttonLabels[15] = std::make_unique<QLabel>("Analog Down");
  buttonLabels[16] = std::make_unique<QLabel>("Analog Left");
  buttonLabels[17] = std::make_unique<QLabel>("Analog Right");

  auto str = JSONGetField<std::string>(settings, "input", "A");
  kbButtons[0] = std::make_unique<QPushButton>(str.c_str());
  str = JSONGetField<std::string>(settings, "input", "B");
  kbButtons[1] = std::make_unique<QPushButton>(str.c_str());
  str = JSONGetField<std::string>(settings, "input", "Z");
  kbButtons[2] = std::make_unique<QPushButton>(str.c_str());
  str = JSONGetField<std::string>(settings, "input", "Start");
  kbButtons[3] = std::make_unique<QPushButton>(str.c_str());
  str = JSONGetField<std::string>(settings, "input", "L");
  kbButtons[4] = std::make_unique<QPushButton>(str.c_str());
  str = JSONGetField<std::string>(settings, "input", "R");
  kbButtons[5] = std::make_unique<QPushButton>(str.c_str());
  str = JSONGetField<std::string>(settings, "input", "Dpad Up");
  kbButtons[6] = std::make_unique<QPushButton>(str.c_str());
  str = JSONGetField<std::string>(settings, "input", "Dpad Down");
  kbButtons[7] = std::make_unique<QPushButton>(str.c_str());
  str = JSONGetField<std::string>(settings, "input", "Dpad Left");
  kbButtons[8] = std::make_unique<QPushButton>(str.c_str());
  str = JSONGetField<std::string>(settings, "input", "Dpad Right");
  kbButtons[9] = std::make_unique<QPushButton>(str.c_str());
  str = JSONGetField<std::string>(settings, "input", "C Up");
  kbButtons[10] = std::make_unique<QPushButton>(str.c_str());
  str = JSONGetField<std::string>(settings, "input", "C Down");
  kbButtons[11] = std::make_unique<QPushButton>(str.c_str());
  str = JSONGetField<std::string>(settings, "input", "C Left");
  kbButtons[12] = std::make_unique<QPushButton>(str.c_str());
  str = JSONGetField<std::string>(settings, "input", "C Right");
  kbButtons[13] = std::make_unique<QPushButton>(str.c_str());
  str = JSONGetField<std::string>(settings, "input", "Analog Up");
  kbButtons[14] = std::make_unique<QPushButton>(str.c_str());
  str = JSONGetField<std::string>(settings, "input", "Analog Down");
  kbButtons[15] = std::make_unique<QPushButton>(str.c_str());
  str = JSONGetField<std::string>(settings, "input", "Analog Left");
  kbButtons[16] = std::make_unique<QPushButton>(str.c_str());
  str = JSONGetField<std::string>(settings, "input", "Analog Right");
  kbButtons[17] = std::make_unique<QPushButton>(str.c_str());

  for (int i = 0; i < 18; i++) {
    connect(kbButtons[i].get(), &QPushButton::pressed, this, [&, i]() {
      devices->setEnabled(false);
      for (const auto &kbButton : kbButtons) {
        kbButton->setEnabled(false);
      }
      grabKeyboard();
      grabbing = true;
      whichGrabbing = i;
    });
  }

  SDL_InitSubSystem(SDL_INIT_GAMEPAD);

  if (SDL_AddGamepadMappingsFromFile("resources/gamecontrollerdb.txt") < 0) {
    Util::warn("[SDL] Could not load game controller DB");
  }

  connect(&refresh, &QTimer::timeout, this, &InputSettings::QueryDevices);
  refresh.start(1000);

  devices->addItem("Keyboard/Mouse");
  deviceComboBoxLayout->addWidget(devicesLabel.get());
  deviceComboBoxLayout->addWidget(devices.get());
  mainLayout->addLayout(deviceComboBoxLayout.get());

  AB->addWidget(buttonLabels[0].get());
  AB->addWidget(kbButtons[0].get());
  AB->addWidget(buttonLabels[1].get());
  AB->addWidget(kbButtons[1].get());
  mainLayout->addLayout(AB.get());
  ZStart->addWidget(buttonLabels[2].get());
  ZStart->addWidget(kbButtons[2].get());
  ZStart->addWidget(buttonLabels[3].get());
  ZStart->addWidget(kbButtons[3].get());
  mainLayout->addLayout(ZStart.get());
  LR->addWidget(buttonLabels[4].get());
  LR->addWidget(kbButtons[4].get());
  LR->addWidget(buttonLabels[5].get());
  LR->addWidget(kbButtons[5].get());
  mainLayout->addLayout(LR.get());
  DupDdown->addWidget(buttonLabels[6].get());
  DupDdown->addWidget(kbButtons[6].get());
  DupDdown->addWidget(buttonLabels[7].get());
  DupDdown->addWidget(kbButtons[7].get());
  mainLayout->addLayout(DupDdown.get());
  DleftDright->addWidget(buttonLabels[8].get());
  DleftDright->addWidget(kbButtons[8].get());
  DleftDright->addWidget(buttonLabels[9].get());
  DleftDright->addWidget(kbButtons[9].get());
  mainLayout->addLayout(DleftDright.get());
  CupCdown->addWidget(buttonLabels[10].get());
  CupCdown->addWidget(kbButtons[10].get());
  CupCdown->addWidget(buttonLabels[11].get());
  CupCdown->addWidget(kbButtons[11].get());
  mainLayout->addLayout(CupCdown.get());
  CleftCright->addWidget(buttonLabels[12].get());
  CleftCright->addWidget(kbButtons[12].get());
  CleftCright->addWidget(buttonLabels[13].get());
  CleftCright->addWidget(kbButtons[13].get());
  mainLayout->addLayout(CleftCright.get());
  AupAdown->addWidget(buttonLabels[14].get());
  AupAdown->addWidget(kbButtons[14].get());
  AupAdown->addWidget(buttonLabels[15].get());
  AupAdown->addWidget(kbButtons[15].get());
  mainLayout->addLayout(AupAdown.get());
  AleftAright->addWidget(buttonLabels[16].get());
  AleftAright->addWidget(kbButtons[16].get());
  AleftAright->addWidget(buttonLabels[17].get());
  AleftAright->addWidget(kbButtons[17].get());
  mainLayout->addLayout(AleftAright.get());
  mainLayout->addStretch();
  setLayout(mainLayout.get());
}


void InputSettings::keyPressEvent(QKeyEvent *e) {
  if (grabbing) {
    const auto k = QKeySequence(e->key()).toString();
    JSONSetField<std::string>(settings, "input", buttonLabels[whichGrabbing]->text().toStdString(), k.toStdString());
    kbButtons[whichGrabbing]->setText(k);
    grabbing = false;
    whichGrabbing = -1;
    devices->setEnabled(true);
    for (const auto &kbButton : kbButtons) {
      kbButton->setEnabled(true);
    }
    releaseKeyboard();
    emit modified();
  }
}

std::array<Qt::Key, 18> InputSettings::GetMappedKeys() const {
  std::array<Qt::Key, 18> ret{};

  for (int i = 0; i < 18; i++) {
    ret[i] = QKeySequence(kbButtons[i]->text().toUpper())[0].key();
  }

  return ret;
}

void InputSettings::QueryDevices() noexcept {
  if (!devices->isEnabled())
    return;

  SDL_Event e;
  while (SDL_PollEvent(&e)) {
    switch (e.type) {
    case SDL_EVENT_GAMEPAD_ADDED:
      {
        const auto index = e.gdevice.which;

        const auto gamepad = SDL_OpenGamepad(index);
        Util::info("Found controller!");
        const auto serial = SDL_GetGamepadSerial(gamepad);
        const auto name = SDL_GetGamepadName(gamepad);
        const auto path = SDL_GetGamepadPath(gamepad);

        if (name) {
          if (!gamepadIndexes.contains(index)) {
            gamepadIndexes[index] = name;
          }
          devices->addItem(name);
        } else if (serial) {
          if (!gamepadIndexes.contains(index)) {
            gamepadIndexes[index] = serial;
          }
          devices->addItem(serial);
        } else if (path) {
          if (!gamepadIndexes.contains(index)) {
            gamepadIndexes[index] = path;
          }
          devices->addItem(path);
        }

        SDL_CloseGamepad(gamepad);
      }
      break;
    case SDL_EVENT_GAMEPAD_REMOVED:
      {
        const auto index = e.gdevice.which;

        if (gamepadIndexes.contains(index))
          devices->removeItem(devices->findText(gamepadIndexes[index].c_str()));
      }
      break;
    }
  }
}
