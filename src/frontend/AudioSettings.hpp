#pragma once
#include <QWidget>
#include <QSlider>
#include <QCheckBox>
#include <JSONUtils.hpp>

class AudioSettings : public QWidget {
  QCheckBox* lockChannels = new QCheckBox;
  Q_OBJECT
public:
  QSlider* volumeL = new QSlider(Qt::Horizontal), * volumeR = new QSlider(Qt::Horizontal);
  AudioSettings(nlohmann::json&);
  nlohmann::json& settings;
Q_SIGNALS:
  void modified();
};