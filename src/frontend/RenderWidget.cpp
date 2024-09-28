#include <Core.hpp>
#include <KaizenQt.hpp>
#include <RenderWidget.hpp>
#include <SDL3/SDL_events.h>

RenderWidget::RenderWidget(const std::shared_ptr<n64::Core> &core) : QWidget(nullptr) {
  setAttribute(Qt::WA_NativeWindow);
  setAttribute(Qt::WA_PaintOnScreen);
  if (GetOSCompositorCategory() == CompositorCategory::Wayland) {
    setAttribute(Qt::WA_DontCreateNativeAncestors);
  }

  if (GetOSCompositorCategory() == CompositorCategory::MacOS) {
    windowHandle()->setSurfaceType(QWindow::MetalSurface);
  } else {
    windowHandle()->setSurfaceType(QWindow::VulkanSurface);
  }

  if (!Vulkan::Context::init_loader(nullptr)) {
    Util::panic("Could not initialize Vulkan ICD");
  }

  qtVkInstanceFactory = std::make_shared<QtInstanceFactory>();
  windowHandle()->setVulkanInstance(&qtVkInstanceFactory->handle);
  windowHandle()->create();

  wsiPlatform = std::make_shared<QtWSIPlatform>(core, windowHandle());
  windowInfo = std::make_shared<QtParallelRdpWindowInfo>(windowHandle());
}

void QtWSIPlatform::poll_input() {
  if (!canPollEvents)
    return;

  SDL_Event e;
  while (SDL_PollEvent(&e)) {
    switch (e.type) {
    case SDL_EVENT_GAMEPAD_ADDED:
      {
        const auto index = e.gdevice.which;

        gamepad = SDL_OpenGamepad(index);
        Util::info("Controller found!");

        const auto serial = SDL_GetGamepadSerial(gamepad);
        const auto name = SDL_GetGamepadName(gamepad);
        const auto path = SDL_GetGamepadPath(gamepad);

        Util::info("\tName: {}", name ? name : "Not available");
        Util::info("\tSerial: {}", serial ? serial : "Not available");
        Util::info("\tPath: {}", path ? path : "Not available");

        gamepadConnected = true;
      }
      break;
    case SDL_EVENT_GAMEPAD_REMOVED:
      {
        gamepadConnected = false;
        SDL_CloseGamepad(gamepad);
      }
      break;
    }
  }

  if (gamepadConnected) {
    n64::PIF &pif = core->cpu->GetMem().mmio.si.pif;
    pif.UpdateButton(0, n64::Controller::Key::A, SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_SOUTH));
    pif.UpdateButton(0, n64::Controller::Key::B, SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_WEST));
    pif.UpdateButton(0, n64::Controller::Key::Z,
                     SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_LEFT_TRIGGER) == SDL_JOYSTICK_AXIS_MAX);
    pif.UpdateButton(0, n64::Controller::Key::Start, SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_START));
    pif.UpdateButton(0, n64::Controller::Key::DUp, SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_DPAD_UP));
    pif.UpdateButton(0, n64::Controller::Key::DDown, SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_DPAD_DOWN));
    pif.UpdateButton(0, n64::Controller::Key::DLeft, SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_DPAD_LEFT));
    pif.UpdateButton(0, n64::Controller::Key::DRight, SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_DPAD_RIGHT));
    pif.UpdateButton(0, n64::Controller::Key::LT, SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_LEFT_SHOULDER));
    pif.UpdateButton(0, n64::Controller::Key::RT, SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER));
    pif.UpdateButton(0, n64::Controller::Key::CUp, SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_RIGHTY) <= -127);
    pif.UpdateButton(0, n64::Controller::Key::CDown, SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_RIGHTY) >= 127);
    pif.UpdateButton(0, n64::Controller::Key::CLeft, SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_RIGHTX) <= -127);
    pif.UpdateButton(0, n64::Controller::Key::CRight, SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_RIGHTX) >= 127);

    float xclamped = SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_LEFTX);
    if (xclamped < 0) {
      xclamped /= float(std::abs(SDL_JOYSTICK_AXIS_MAX));
    } else {
      xclamped /= SDL_JOYSTICK_AXIS_MAX;
    }

    xclamped *= 86;

    float yclamped = SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_LEFTY);
    if (yclamped < 0) {
      yclamped /= float(std::abs(SDL_JOYSTICK_AXIS_MIN));
    } else {
      yclamped /= SDL_JOYSTICK_AXIS_MAX;
    }

    yclamped *= 86;

    pif.UpdateAxis(0, n64::Controller::Axis::Y, -yclamped);
    pif.UpdateAxis(0, n64::Controller::Axis::X, xclamped);
  }
}
