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

  buttonBox = new QDialogButtonBox(QDialogButtonBox::Cancel | QDialogButtonBox::Apply);

  buttonBox->button(QDialogButtonBox::Apply)->setEnabled(false);

  connect(cpuSettings, &CPUSettings::modified, this, [&]() {
    buttonBox->button(QDialogButtonBox::Apply)->setEnabled(true);
  });

  connect(audioSettings, &AudioSettings::modified, this, [&]() {
    buttonBox->button(QDialogButtonBox::Apply)->setEnabled(true);
  });

  connect(inputSettings, &InputSettings::modified, this, [&]() {
    buttonBox->button(QDialogButtonBox::Apply)->setEnabled(true);
  });

  connect(buttonBox->button(QDialogButtonBox::Apply), &QPushButton::pressed, this, [&]() {
    auto newMap = inputSettings->GetMappedKeys();
    if (!std::equal(keyMap.begin(), keyMap.end(), newMap.begin(), newMap.end())) {
      keyMap = newMap;
      emit regrabKeyboard();
    }
    buttonBox->button(QDialogButtonBox::Apply)->setEnabled(false);
    std::ofstream file("resources/settings.json");
    file << settings;
    file.close();
  });

  connect(buttonBox->button(QDialogButtonBox::Cancel), &QPushButton::pressed, this, &QWidget::hide);

  QVBoxLayout* mainLayout = new QVBoxLayout;
  QHBoxLayout* buttonsLayout = new QHBoxLayout;
  buttonsLayout->addWidget(buttonBox);
  mainLayout->addWidget(tabs);
  mainLayout->addLayout(buttonsLayout);
  setLayout(mainLayout);
}