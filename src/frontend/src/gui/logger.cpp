#include <gui/logger.hpp>
#include <gui.hpp>

Logger::Logger() {
  logger.InfoStr = message_type_strings[0].c_str();
  logger.WarnStr = message_type_strings[1].c_str();
  logger.ErrorStr = message_type_strings[2].c_str();
}

void Logger::LogWindow(Gui* gui) {
  std::string final_message{};
  if(last_message != nullptr && strcmp(last_message, "") && strcmp(last_message, gui->old_message.c_str())) {
    if(last_message_type == FATAL) {
      if(gui->pause_on_fatal) {
        gui->core.running = false;
      } else {
        gui->Stop();
      }
    }

    gui->old_message = std::string(last_message);
    gui->old_message_type = last_message_type;
    final_message = message_type_strings[last_message_type] + " " + gui->old_message;
    logger.AddLog("%s", final_message.c_str());
  }
  logger.Draw("Logs", &gui->show_logs);
}