#include <AudioSettings.hpp>
#include <QLabel>
#include <QVBoxLayout>

AudioSettings::AudioSettings(nlohmann::json &settings) : settings(settings), QWidget(nullptr) {
  lockChannels->setChecked(JSONGetField<bool>(settings, "audio", "lock"));
  volumeL->setValue(JSONGetField<float>(settings, "audio", "volumeL") * 100);
  volumeR->setValue(JSONGetField<float>(settings, "audio", "volumeR") * 100);
  volumeL->setRange(0, 100);
  volumeR->setRange(0, 100);

  connect(lockChannels, &QCheckBox::stateChanged, this, [&]() {
    JSONSetField(settings, "audio", "lock", lockChannels->isChecked());
    if (lockChannels->isChecked()) {
      volumeR->setValue(volumeL->value());
    }

    emit modified();
  });

  connect(volumeL, &QSlider::valueChanged, this, [&]() {
    JSONSetField(settings, "audio", "volumeL", float(volumeL->value()) / 100.f);
    if (lockChannels->isChecked()) {
      volumeR->setValue(volumeL->value());
      JSONSetField(settings, "audio", "volumeR", float(volumeL->value()) / 100.f);
    }
    emit modified();
  });

  connect(volumeR, &QSlider::valueChanged, this, [&]() {
    if (!lockChannels->isChecked()) {
      JSONSetField(settings, "audio", "volumeR", float(volumeR->value()) / 100.f);
    }
    emit modified();
  });

  auto labelLock = new QLabel("Lock channels:");
  auto labelL = new QLabel("Volume L");
  auto labelR = new QLabel("Volume R");

  auto mainLayout = new QVBoxLayout;
  auto volLayout = new QHBoxLayout;
  mainLayout->addWidget(labelLock);
  mainLayout->addWidget(lockChannels);
  volLayout->addWidget(labelL);
  volLayout->addWidget(volumeL);
  volLayout->addWidget(labelR);
  volLayout->addWidget(volumeR);
  mainLayout->addLayout(volLayout);
  mainLayout->addStretch();
  setLayout(mainLayout);
}
