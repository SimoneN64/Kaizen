#pragma once
#include <log.h>
#include <imgui.h>
#include <imgui_logger.h>
#include <string>

static const std::string message_type_strings[3] = {"[INFO]", "[WARNING]", "[FATAL]"};

struct Gui;

struct Logger {
  Logger();
  void LogWindow(Gui* gui);
  ImGui::Logger logger;
};