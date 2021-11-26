#include <core.h>
#include <gui.hpp>
#include <utils.h>
#include <string>
#include <cstring>

#define GLSL_VERSION "#version 130"

INLINE void* core_callback(void* args) {
  Gui* gui = (Gui*)args;
  while(!atomic_load(&gui->emu_quit)) {
    clock_t begin = clock();
    run_frame(&gui->core);
    clock_t end = clock();
    gui->delta += end - begin;
  }
  
  return NULL;
}

Gui::~Gui() {
  free(rom_file);
  emu_quit = true;
  pthread_join(emu_thread, NULL);
  destroy_core(&core);
  NFD_Quit();
}

Gui::Gui(const char* title) : context(title, GLSL_VERSION) {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();

  ImGuiIO& io = ImGui::GetIO(); (void)io;
  ImGuiStyle& style = ImGui::GetStyle();

  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

  ImGui::StyleColorsDark();

  style.WindowRounding = 10;

  const char* glsl_version = GLSL_VERSION;
  
  ImGui_ImplSDL2_InitForOpenGL(context.window, context.gl_context);
  ImGui_ImplOpenGL3_Init(glsl_version);

  io.Fonts->AddFontFromFileTTF("resources/FiraCode-VariableFont_wght.ttf", 16);
  
  init_core(&core);

  pthread_create(&emu_thread, NULL, core_callback, (void*)this);

  NFD_Init();
}

void Gui::MainLoop() {
  ImGuiIO& io = ImGui::GetIO(); (void)io;
  unsigned int frames = 0;
  while(running) {    
    SDL_Event event;
    while(SDL_PollEvent(&event)) {
      ImGui_ImplSDL2_ProcessEvent(&event);

      switch(event.type) {
        case SDL_QUIT: running = false; break;
        case SDL_WINDOWEVENT:
          if(event.window.event == SDL_WINDOWEVENT_CLOSE) {
            running = false;
          }
          break;  
        case SDL_KEYDOWN:
          switch(event.key.keysym.sym) {
            case SDLK_o: OpenFile(); break;
            case SDLK_p:
              if(rom_loaded) {
                core.running = !core.running;
              }
              break;
            case SDLK_ESCAPE: running = false; break;
          }
          break;
      }
    }

    context.Frame();
    
    if(show_metrics) ImGui::ShowMetricsWindow(&show_metrics);
    DebuggerWindow();

    context.MainWindow(this, &core);

    frames++;

    if(clock_to_ms(delta) > 1000.0) {
      fps = (double)frames * 0.5 + fps *0.5;
      frames = 0;
      delta -= CLOCKS_PER_SEC;
      frametime = 1000.0 / ((fps == 0) ? 0.001 : fps);
    }

    context.Update(io);
  }
}

void Gui::MainMenubar() {
  ImVec2 window_size = ImGui::GetWindowSize();
  if(ImGui::BeginMenuBar()) {
    if(ImGui::BeginMenu("File")) {
      if(ImGui::MenuItem("Open", "O")) {
        OpenFile();
      }

      if(ImGui::MenuItem("Exit", "Esc")) {
        running = false;
      }
      ImGui::EndMenu();
    }

    if(ImGui::BeginMenu("Emulation")) {
      std::string pause_text = "Pause";
      if(!core.running && rom_loaded) {
        pause_text = "Resume";
      }

      if(ImGui::MenuItem(pause_text.c_str(), "P", false, rom_loaded)) {
        core.running = !core.running;
        if(core.running) {
          core.stepping = false;
        }
      }

      if(ImGui::MenuItem("Stop", NULL, false, rom_loaded)) {
        Stop();
      }

      if(ImGui::MenuItem("Reset", NULL, false, rom_loaded)) {
        Reset();
      }

      ImGui::EndMenu();
    }

    if(ImGui::BeginMenu("Settings")) {
      ImGui::MenuItem("Show disassembly", NULL, &show_disasm, true);
      ImGui::MenuItem("Show register watch", NULL, &show_regs, true);
      ImGui::MenuItem("Show logs", NULL, &show_logs, true);
      ImGui::MenuItem("Show metrics", NULL, &show_metrics, true);
      ImGui::EndMenu();
    }

    ImVec2 close_button_size = ImGui::CalcTextSize("[X]");
    char fps_text[255];
    sprintf(fps_text, rom_loaded ? "[ %.2f fps ][ %.2f ms ]" : "[ NaN fps ][ NaN ms ]", fps, frametime);
    ImVec2 fps_size = ImGui::CalcTextSize(fps_text);
    ImGuiStyle& style = ImGui::GetStyle();
    ImGui::SameLine(window_size.x - close_button_size.x - fps_size.x - style.ItemInnerSpacing.x * 4 - 12, -1);
    ImGui::Text("%s", fps_text);
    ImGui::SameLine(window_size.x - close_button_size.x - style.ItemInnerSpacing.x * 2 - 12, -1);
    
    if(ImGui::BeginMenu("[X]")) {
      running = false;
      ImGui::EndMenu();
    }

    ImGui::EndMenuBar();
  }
}

void Gui::RegistersView() {
  registers_t* regs = &core.cpu.regs;
  ImGui::Begin("Registers view", &show_regs, 0);
  for(int i = 0; i < 32; i+=4) {
    ImGui::Text("%s: %016lX %s: %016lX %s: %016lX %s: %016lX", regs_str[i], regs->gpr[i], regs_str[i + 1], regs->gpr[i + 1], regs_str[i + 2], regs->gpr[i + 2], regs_str[i + 3], regs->gpr[i + 3]);
  }
  ImGui::Separator();
  s64 pipe[3] = {regs->old_pc, regs->pc, regs->next_pc};
  for(int i = 0; i < 3; i++) {
    ImGui::Text("pipe[%d]: %016lX", i, pipe[i]);
  }
  ImGui::End();
}

void Gui::DebuggerWindow() {
  if(show_regs) RegistersView();
  if(show_logs) logger.LogWindow(this);
}

void Gui::OpenFile() {
  nfdfilteritem_t filter = { "Nintendo 64 roms", "n64,z64,v64,N64,Z64,V64" };
  nfdresult_t result = NFD_OpenDialog(&rom_file, &filter, 1, EMU_DIR);
  if(result == NFD_OKAY) {
    Reset();
  }
}

void Gui::Start() {
  rom_loaded = load_rom(&core.mem, rom_file);
  emu_quit = !rom_loaded;
  core.running = rom_loaded;
  if(rom_loaded) {
    pthread_create(&emu_thread, NULL, core_callback, (void*)this);
  }
}

void Gui::Reset() {
  Stop();
  Start();
}

void Gui::Stop() {
  emu_quit = true;
  pthread_join(emu_thread, NULL);
  init_core(&core);
  rom_loaded = false;
  core.running = false;
}