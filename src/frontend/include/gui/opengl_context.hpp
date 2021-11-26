#pragma once
#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_opengl.h>
#include <core.h>
#include <imgui.h>

struct Gui;

struct GLData {
  u32 old_w, old_h;
  u8 old_format = 14;
  int glFormat = GL_UNSIGNED_SHORT_5_5_5_1;
  u8 depth = 2;
};

struct OpenGLContext {
  GLData gl_data;
	SDL_Window* window;
  unsigned int id; // OpenGL framebuffer texture ID
  u8* framebuffer;
  SDL_GLContext gl_context;
  OpenGLContext(const char* title, const char* m_glsl_version);
  void UpdateTexture(core_t* core);
  void MainWindow(Gui* gui, core_t* core);
  void Update(ImGuiIO& io);
  void Frame();
  ~OpenGLContext();
};