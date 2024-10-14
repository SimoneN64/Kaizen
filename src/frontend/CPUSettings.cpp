#include <CPUSettings.hpp>
#include <JSONUtils.hpp>
#include <log.hpp>

CPUSettings::CPUSettings(nlohmann::json &settings) : QWidget(nullptr), settings(settings) {
  cpuTypes->addItems({
    "Interpreter" //, "Dynamic Recompiler"
  });
  if (JSONGetField<std::string>(settings, "cpu", "type") == "jit") {
    cpuTypes->setCurrentIndex(1);
  } else {
    cpuTypes->setCurrentIndex(0);
  }

  connect(cpuTypes.get(), &QComboBox::currentIndexChanged, this, [&]() {
    if (cpuTypes->currentIndex() == 0) {
      JSONSetField(settings, "cpu", "type", "interpreter");
      //} else if (cpuTypes->currentIndex() == 1) {
      //  JSONSetField(settings, "cpu", "type", "jit");
    } else {
      Util::panic("Impossible CPU type!");
    }

    emit modified();
  });

  mainLayout->addWidget(label.get());
  mainLayout->addWidget(cpuTypes.get());
  mainLayout->addStretch();
  setLayout(mainLayout.get());
}
