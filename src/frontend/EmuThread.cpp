#include <EmuThread.hpp>

EmuThread::EmuThread(QObject* parent_object) noexcept : QThread(parent_object) {}

[[noreturn]] void EmuThread::run() noexcept {
  while (true) {
    if (!core.pause) {
      core.Run(0.5, 0.5);
    }
  }
}