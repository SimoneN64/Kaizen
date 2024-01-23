#include <CPUSettings.hpp>
#include <QVBoxLayout>
#include <QLabel>
#include <JSONUtils.hpp>
#include <log.hpp>

CPUSettings::CPUSettings(nlohmann::json& settings) : settings(settings), QWidget(nullptr) {
  cpuTypes->addItems({ "Interpreter", "Dynamic Recompiler" });

  if (JSONGetField<std::string>(settings, "cpu", "type") == "jit") {
    cpuTypes->setCurrentIndex(1);
  } else {
    cpuTypes->setCurrentIndex(0);
  }

  connect(cpuTypes, &QComboBox::currentIndexChanged, this, [&]() {
    if (cpuTypes->currentIndex() == 0) {
      JSONSetField(settings, "cpu", "type", "interpreter");
    } else if (cpuTypes->currentIndex() == 1) {
      JSONSetField(settings, "cpu", "type", "jit");
    } else {
      Util::panic("Impossible CPU type!");
    }

    emit modified();
  });

  QLabel* label = new QLabel("CPU type:");

  QVBoxLayout* mainLayout = new QVBoxLayout;
  mainLayout->addWidget(label);
  mainLayout->addWidget(cpuTypes);
  mainLayout->addStretch();
  setLayout(mainLayout);
}