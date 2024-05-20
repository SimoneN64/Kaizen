#include <SettingsWindow.hpp>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QButtonGroup>
#include <QFileDialog>
#include <fmt/core.h>

std::string savePath = "";

SettingsWindow::SettingsWindow() : QWidget(nullptr) {
  settings = JSONOpenOrCreate("resources/settings.json");

  savePath = JSONGetField<std::string>(settings, "general", "savePath");

  if (objectName().isEmpty())
    setObjectName("Settings");

  resize(500, 400);
  setWindowTitle("Settings");

  cpuSettings = new CPUSettings(settings);
  audioSettings = new AudioSettings(settings);
  inputSettings = new InputSettings(settings);
  generalSettings = new QWidget;
  keyMap = inputSettings->GetMappedKeys();

  folderLabelPrefix = new QLabel("Save files' path: ");
  folderLabel = new QLabel(fmt::format("{}", savePath).c_str());

  connect(folderBtn, &QPushButton::pressed, this, [&]() {
    savePath = QFileDialog::getExistingDirectory(this, tr("Select directory")).toStdString();
    folderLabel->setText(fmt::format("{}", savePath).c_str());
    JSONSetField(settings, "general", "savePath", savePath);
    apply->setEnabled(true);
  });

  auto generalLayout = new QHBoxLayout;
  auto generalLayoutV = new QVBoxLayout;
  generalLayout->addWidget(folderLabelPrefix);
  generalLayout->addWidget(folderLabel);
  generalLayout->addStretch();
  generalLayout->addWidget(folderBtn);
  generalLayoutV->addLayout(generalLayout);
  generalLayoutV->addStretch();
  generalSettings->setLayout(generalLayoutV);

  auto* tabs = new QTabWidget;
  tabs->addTab(generalSettings, tr("General"));
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