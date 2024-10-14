#pragma once
#include <JSONUtils.hpp>
#include <QCheckBox>
#include <QSlider>
#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>

class AudioSettings final : public QWidget {
  std::unique_ptr<QCheckBox> lockChannels = std::make_unique<QCheckBox>();
  std::unique_ptr<QLabel> labelLock = std::make_unique<QLabel>("Lock channels:");
  std::unique_ptr<QLabel> labelL = std::make_unique<QLabel>("Volume L");
  std::unique_ptr<QLabel> labelR = std::make_unique<QLabel>("Volume R");
  std::unique_ptr<QVBoxLayout> mainLayout = std::make_unique<QVBoxLayout>();
  std::unique_ptr<QHBoxLayout> volLayout = std::make_unique<QHBoxLayout>();
  Q_OBJECT
public:
  std::unique_ptr<QSlider> volumeL = std::make_unique<QSlider>(Qt::Horizontal),
                           volumeR = std::make_unique<QSlider>(Qt::Horizontal);
  explicit AudioSettings(nlohmann::json &);
  nlohmann::json &settings;
Q_SIGNALS:
  void modified();
};
