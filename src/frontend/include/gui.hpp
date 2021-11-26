#pragma once
#include <core.h>
#include <imgui.h>
#include <imgui_impl_sdl.h>
#include <imgui_impl_opengl3.h>
#include <nfd.hpp>
#include <atomic>
#include <pthread.h>
#include <ctime>
#include <gui/logger.hpp>
#include <gui/opengl_context.hpp>

static const ImVec4 colors_disasm[3] = {ImVec4(1.0, 0.000, 0.0, 1.0),  // RED
                                        ImVec4(1.0, 0.619, 0.0, 1.0),  // ORANGE
                                        ImVec4(1.0, 0.988, 0.0, 1.0)}; // YELLOW
static const ImVec4 clear_color = {0.45f, 0.55f, 0.60f, 1.00f};

struct Gui {
  ImGuiContext* ctx;
  bool show_disasm = true, show_regs = true;
  bool show_metrics = false, show_logs = true;
  bool pause_on_fatal = true;
  float log_pos_y = 0;
  std::string old_message = "NULL";
  message_type old_message_type = INFO;
	nfdchar_t* rom_file;
  bool rom_loaded = false, running = true;
  pthread_t emu_thread;
  Logger logger;
  clock_t delta;
  double fps = 60.0, frametime = 16.0;
  std::atomic_bool emu_quit = false;
  core_t core;
  OpenGLContext context;
  
  Gui(const char* title);
  ~Gui();
  void MainLoop();
  void DestroyGui();
  void OpenFile();
  void MainMenubar();
  void DebuggerWindow();
  void RegistersView();
  void Reset();
  void Stop();
  void Start();
};