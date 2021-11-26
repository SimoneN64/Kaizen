#include <gui/opengl_context.hpp>
#include <log.h>
#include <gui.hpp>

OpenGLContext::~OpenGLContext() {
  free(framebuffer);
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImGui::DestroyContext();
  SDL_GL_DeleteContext(gl_context);
  SDL_DestroyWindow(window);
  SDL_Quit();
}

OpenGLContext::OpenGLContext(const char* title, const char* m_glsl_version) {
  if(SDL_Init(SDL_INIT_VIDEO) != 0) {
    logfatal("Error: %s\n", SDL_GetError());
  }

  const char* glsl_version = m_glsl_version;
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
  
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

  SDL_DisplayMode mode;
  SDL_GetCurrentDisplayMode(0, &mode);
  int w = mode.w - (mode.w / 4), h = mode.h - (mode.h / 4);
  
  window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, w, h, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
  gl_context = SDL_GL_CreateContext(window);
  SDL_GL_MakeCurrent(window, gl_context);
  SDL_GL_SetSwapInterval(0); // Enable vsync

  framebuffer = (u8*)malloc(320 * 240 * 4);
  memset(framebuffer, 0x000000ff, 320 * 240 * 4);

  glGenTextures(1, &id);
  glBindTexture(GL_TEXTURE_2D, id);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 320, 240, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, framebuffer);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
}

ImVec2 image_size;

INLINE void resize_callback(ImGuiSizeCallbackData* data) {
  ImVec2 window_size = ImGui::GetWindowSize();
  float x = window_size.x - 15, y = window_size.y - 15;
  float current_aspect_ratio = x / y;

  if(N64_ASPECT_RATIO > current_aspect_ratio) {
    y = x / (N64_ASPECT_RATIO);
  } else {
    x = y * (N64_ASPECT_RATIO);
  }

  image_size.x = x;
  image_size.y = y - 30;
}

void OpenGLContext::UpdateTexture(core_t* core) {
  u32 w = core->mem.mmio.vi.width, h = 0.75 * w;
  u32 origin = core->mem.mmio.vi.origin & 0xFFFFFF;
  u8 format = core->mem.mmio.vi.status.format;
  bool reconstruct_texture = false;
  bool res_changed = gl_data.old_w != w || gl_data.old_h != h;
  bool format_changed = gl_data.old_format != format;

  if(res_changed) {
    gl_data.old_w = w;
    gl_data.old_h = h;

    reconstruct_texture = true;
  }

  if(format_changed) {
    gl_data.old_format = format;
    if(format == f5553) {
      gl_data.glFormat = GL_UNSIGNED_SHORT_5_5_5_1;
      gl_data.depth = 2;
    } else if (format == f8888) {
      gl_data.glFormat = GL_UNSIGNED_INT_8_8_8_8;
      gl_data.depth = 4;
    }

    reconstruct_texture = true;
  }

  if(reconstruct_texture) {
    framebuffer = (u8*)realloc(framebuffer, w * h * gl_data.depth);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, gl_data.glFormat, framebuffer);
  }

  if(format == f8888) {
    framebuffer[4] = 0xff;
    memcpy(framebuffer, &core->mem.rdram[origin & RDRAM_DSIZE], w * h * gl_data.depth);
    for(int i = 0; i < w * h * gl_data.depth; i += gl_data.depth) {
      framebuffer[i + 4] |= 0xff;
    }
  } else {
    framebuffer[1] |= 1;
    for(int i = 0; i < w * h * gl_data.depth; i += gl_data.depth) {
      framebuffer[i] = core->mem.rdram[HALF_ADDR(origin + i & RDRAM_DSIZE)];
      framebuffer[i + 1] = core->mem.rdram[HALF_ADDR(origin + 1 + i & RDRAM_DSIZE)] | (1 << 16);
    }
  }

  glBindTexture(GL_TEXTURE_2D, id);
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_RGBA, gl_data.glFormat, framebuffer);
}

void OpenGLContext::MainWindow(Gui* gui, core_t *core) {
  ImGui::SetNextWindowSizeConstraints((ImVec2){0, 0}, (ImVec2){__FLT_MAX__, __FLT_MAX__}, resize_callback, NULL);
  ImGui::Begin("Display", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking);
  gui->MainMenubar();
  UpdateTexture(core);
  ImVec2 window_size = ImGui::GetWindowSize();
  ImVec2 result = {static_cast<float>((window_size.x - image_size.x) * 0.5), static_cast<float>((window_size.y - image_size.y + 15) * 0.5)};
  ImGui::SetCursorPos(result);
  ImGui::Image((ImTextureID)((intptr_t)id), image_size, (ImVec2){0, 0}, (ImVec2){1, 1}, (ImVec4){1, 1, 1, 1}, (ImVec4){0, 0, 0, 0});
  ImGui::End();
}

void OpenGLContext::Frame() {
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplSDL2_NewFrame();
  ImGui::NewFrame();
}

void OpenGLContext::Update(ImGuiIO& io) {
  ImGui::Render();

  glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
  glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
  glClear(GL_COLOR_BUFFER_BIT);
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

  SDL_GL_SwapWindow(window);
}