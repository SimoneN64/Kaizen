#include <SettingsWindow.hpp>
#include <fmt/core.h>

std::string savePath = "";

SettingsWindow::SettingsWindow() : QWidget(nullptr) {
  settings = JSONOpenOrCreate("resources/settings.json");

  savePath = JSONGetField<std::string>(settings, "general", "savePath");

  if (objectName().isEmpty())
    setObjectName("Settings");

  resize(500, 400);
  setWindowTitle("Settings");

  cpuSettings = std::make_unique<CPUSettings>(settings);
  audioSettings = std::make_unique<AudioSettings>(settings);
  inputSettings = std::make_unique<InputSettings>(settings);
  generalSettings = std::make_unique<QWidget>();
  keyMap = inputSettings->GetMappedKeys();

  folderLabel = std::make_unique<QLabel>(fmt::format("{}", savePath).c_str());

  connect(folderBtn.get(), &QPushButton::pressed, this, [&]() {
    savePath = QFileDialog::getExistingDirectory(this, tr("Select directory")).toStdString();
    folderLabel->setText(fmt::format("{}", savePath).c_str());
    JSONSetField(settings, "general", "savePath", savePath);
    apply->setEnabled(true);
  });

  generalLayout->addWidget(folderLabelPrefix.get());
  generalLayout->addWidget(folderLabel.get());
  generalLayout->addStretch();
  generalLayout->addWidget(folderBtn.get());
  generalLayoutV->addLayout(generalLayout.get());
  generalLayoutV->addStretch();
  generalSettings->setLayout(generalLayoutV.get());

  tabs->addTab(generalSettings.get(), tr("General"));
  tabs->addTab(cpuSettings.get(), tr("CPU"));
  tabs->addTab(audioSettings.get(), tr("Audio"));
  tabs->addTab(inputSettings.get(), tr("Input"));

  apply->setEnabled(false);

  connect(cpuSettings.get(), &CPUSettings::modified, this, [&]() { apply->setEnabled(true); });

  connect(audioSettings.get(), &AudioSettings::modified, this, [&]() { apply->setEnabled(true); });

  connect(inputSettings.get(), &InputSettings::modified, this, [&]() { apply->setEnabled(true); });

  connect(apply.get(), &QPushButton::pressed, this, [&]() {
    auto newMap = inputSettings->GetMappedKeys();
    if (!std::ranges::equal(keyMap, newMap)) {
      keyMap = newMap;
      emit regrabKeyboard();
    }
    apply->setEnabled(false);
    std::ofstream file("resources/settings.json");
    file << settings;
    file.close();
  });

  connect(cancel.get(), &QPushButton::pressed, this, &QWidget::hide);

  buttonsLayout->addWidget(apply.get());
  buttonsLayout->addWidget(cancel.get());
  mainLayout->addWidget(tabs.get());
  mainLayout->addLayout(buttonsLayout.get());
  setLayout(mainLayout.get());
}
