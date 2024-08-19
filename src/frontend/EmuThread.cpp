#include <EmuThread.hpp>
#include <SDL2/SDL.h>

EmuThread::EmuThread(const std::shared_ptr<QtInstanceFactory>& instance_, const std::shared_ptr<Vulkan::WSIPlatform>& wsiPlatform_, const std::shared_ptr<ParallelRDP::WindowInfo>& windowInfo_, SettingsWindow& settings) noexcept
  : instance(instance_), wsiPlatform(wsiPlatform_),
    windowInfo(windowInfo_),
    core(parallel), settings(settings) {}

[[noreturn]] void EmuThread::run() noexcept {
  parallel.Init(instance, wsiPlatform, windowInfo, core.cpu->GetMem().GetRDRAMPtr());
  SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER);
  bool controllerConnected = false;

  if(SDL_GameControllerAddMappingsFromFile("resources/gamecontrollerdb.txt") < 0) {
    Util::warn("[SDL] Could not load game controller DB");
  }

  auto pollEvents = [&]() {
    SDL_Event e;
    while(SDL_PollEvent(&e)) {
      switch(e.type) {
        case SDL_CONTROLLERDEVICEADDED: {
          const int index = e.cdevice.which;
          controller = SDL_GameControllerOpen(index);
          Util::info("Found controller!");
          auto serial = SDL_GameControllerGetSerial(controller);
          auto name = SDL_GameControllerName(controller);
          auto path = SDL_GameControllerPath(controller);
          Util::info("\tName: {}", name ? name : "Not available");
          Util::info("\tSerial: {}", serial ? serial : "Not available");
          Util::info("\tPath: {}", path ? path : "Not available");
          controllerConnected = true;
        } break;
        case SDL_CONTROLLERDEVICEREMOVED: {
          controllerConnected = false;
          SDL_GameControllerClose(controller);
        } break;
      }
    }
  };

  while (true) {
    if (!core.pause) {
      core.Run(settings.getVolumeL(), settings.getVolumeR());
      if(core.render) {
        parallel.UpdateScreen(core.cpu->GetMem().mmio.vi);
      }
    } else {
      if(core.render) {
        parallel.UpdateScreen(core.cpu->GetMem().mmio.vi, true);
      }
    }

    pollEvents();

    if(controllerConnected) {
      n64::PIF& pif = core.cpu->GetMem().mmio.si.pif;
      pif.UpdateButton(0, n64::Controller::Key::A, SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_A));
      pif.UpdateButton(0, n64::Controller::Key::B, SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_X));
      pif.UpdateButton(0, n64::Controller::Key::Z, SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_TRIGGERLEFT) == SDL_JOYSTICK_AXIS_MAX);
      pif.UpdateButton(0, n64::Controller::Key::Start, SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_START));
      pif.UpdateButton(0, n64::Controller::Key::DUp, SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_UP));
      pif.UpdateButton(0, n64::Controller::Key::DDown, SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_DOWN));
      pif.UpdateButton(0, n64::Controller::Key::DLeft, SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_LEFT));
      pif.UpdateButton(0, n64::Controller::Key::DRight, SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_RIGHT));
      pif.UpdateButton(0, n64::Controller::Key::LT, SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_LEFTSHOULDER));
      pif.UpdateButton(0, n64::Controller::Key::RT, SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER));
      pif.UpdateButton(0, n64::Controller::Key::CUp, SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_RIGHTY) <= -127);
      pif.UpdateButton(0, n64::Controller::Key::CDown, SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_RIGHTY) >= 127);
      pif.UpdateButton(0, n64::Controller::Key::CLeft, SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_RIGHTX) <= -127);
      pif.UpdateButton(0, n64::Controller::Key::CRight, SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_RIGHTX) >= 127);

      float xclamped = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTX);
      if(xclamped < 0) {
        xclamped /= float(std::abs(SDL_JOYSTICK_AXIS_MIN));
      } else {
        xclamped /= SDL_JOYSTICK_AXIS_MAX;
      }

      xclamped *= 86;

      float yclamped = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTY);
      if(yclamped < 0) {
        yclamped /= float(std::abs(SDL_JOYSTICK_AXIS_MIN));
      } else {
        yclamped /= SDL_JOYSTICK_AXIS_MAX;
      }

      yclamped *= 86;

      pif.UpdateAxis(0, n64::Controller::Axis::Y, -yclamped);
      pif.UpdateAxis(0, n64::Controller::Axis::X, xclamped);
    }
  }
}