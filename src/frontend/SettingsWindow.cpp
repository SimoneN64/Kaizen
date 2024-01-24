#include <SettingsWindow.hpp>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QButtonGroup>

SettingsWindow::SettingsWindow() : QWidget(nullptr) {
  settings = JSONOpenOrCreate("resources/settings.json");

  if (objectName().isEmpty())
    setObjectName("Settings");

  resize(500, 400);
  setWindowTitle("Settings");

  cpuSettings = new CPUSettings(settings);
  audioSettings = new AudioSettings(settings);
  inputSettings = new InputSettings(settings);
  keyMap = inputSettings->GetMappedKeys();

  auto* tabs = new QTabWidget;
  tabs->addTab(cpuSettings, tr("CPU"));
  tabs->addTab(audioSettings, tr("Audio"));
  tabs->addTab(inputSettings, tr("Input"));

  apply->setEnabled(false);

  connect(cpuSettings, &CPUSettings::modified, this, [&]() {
    apply->setEnabled(true);
  });

  connect(audioSettings, &AudioSettings::modified, this, [&]() {
    apply->setEnabled(true);
  });

  connect(inputSettings, &InputSettings::modified, this, [&]() {
    apply->setEnabled(true);
  });

  connect(apply, &QPushButton::pressed, this, [&]() {
    auto newMap = inputSettings->GetMappedKeys();
    if (!std::equal(keyMap.begin(), keyMap.end(), newMap.begin(), newMap.end())) {
      keyMap = newMap;
      emit regrabKeyboard();
    }
    apply->setEnabled(false);
    std::ofstream file("resources/settings.json");
    file << settings;
    file.close();
  });

  connect(cancel, &QPushButton::pressed, this, &QWidget::hide);

  QVBoxLayout* mainLayout = new QVBoxLayout;
  QHBoxLayout* buttonsLayout = new QHBoxLayout;
  buttonsLayout->addWidget(apply);
  buttonsLayout->addWidget(cancel);
  mainLayout->addWidget(tabs);
  mainLayout->addLayout(buttonsLayout);
  setLayout(mainLayout);
}