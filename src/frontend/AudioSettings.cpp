#include <AudioSettings.hpp>

AudioSettings::AudioSettings(nlohmann::json &settings) : settings(settings), QWidget(nullptr) {
  lockChannels->setChecked(JSONGetField<bool>(settings, "audio", "lock"));
  volumeL->setValue(JSONGetField<float>(settings, "audio", "volumeL") * 100);
  volumeR->setValue(JSONGetField<float>(settings, "audio", "volumeR") * 100);
  volumeL->setRange(0, 100);
  volumeR->setRange(0, 100);

  connect(lockChannels.get(), &QCheckBox::stateChanged, this, [&]() {
    JSONSetField(settings, "audio", "lock", lockChannels->isChecked());
    if (lockChannels->isChecked()) {
      volumeR->setValue(volumeL->value());
    }

    emit modified();
  });

  connect(volumeL.get(), &QSlider::valueChanged, this, [&]() {
    JSONSetField(settings, "audio", "volumeL", float(volumeL->value()) / 100.f);
    if (lockChannels->isChecked()) {
      volumeR->setValue(volumeL->value());
      JSONSetField(settings, "audio", "volumeR", float(volumeL->value()) / 100.f);
    }
    emit modified();
  });

  connect(volumeR.get(), &QSlider::valueChanged, this, [&]() {
    if (!lockChannels->isChecked()) {
      JSONSetField(settings, "audio", "volumeR", float(volumeR->value()) / 100.f);
    }
    emit modified();
  });

  mainLayout->addWidget(labelLock.get());
  mainLayout->addWidget(lockChannels.get());
  volLayout->addWidget(labelL.get());
  volLayout->addWidget(volumeL.get());
  volLayout->addWidget(labelR.get());
  volLayout->addWidget(volumeR.get());
  mainLayout->addLayout(volLayout.get());
  mainLayout->addStretch();
  setLayout(mainLayout.get());
}
