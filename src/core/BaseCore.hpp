#pragma once

namespace natsukashii::core {
struct BaseCore {
  virtual ~BaseCore() {}
  virtual void Run();
};
}
