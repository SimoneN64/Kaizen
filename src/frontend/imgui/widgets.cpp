#include <Window.hpp>
#include <nfd.h>
#include <debugger.hpp>

void Window::MainMenuBar(n64::Core& core) {
  ImGui::PushFont(uiFont);
  if(windowID == SDL_GetWindowID(SDL_GetMouseFocus())) {
    ImGui::BeginMainMenuBar();
    if (ImGui::BeginMenu("File")) {
      if (ImGui::MenuItem("Open", "O")) {
        nfdchar_t *outpath;
        const nfdu8filteritem_t filter{"Nintendo 64 roms", "n64,z64,v64,N64,Z64,V64"};
        nfdresult_t result = NFD_OpenDialog(&outpath, &filter, 1, nullptr);
        if (result == NFD_OKAY) {
          core.LoadROM(outpath);
          NFD_FreePath(outpath);
        }
      }
      if (ImGui::MenuItem("Exit")) {
        core.done = true;
      }
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Emulation")) {
      if (ImGui::MenuItem("Reset")) {
        core.Reset();
      }
      if (ImGui::MenuItem("Stop")) {
        core.Stop();
      }
      if (ImGui::MenuItem(core.pause ? "Resume" : "Pause", nullptr, false, core.romLoaded)) {
        core.TogglePause();
      }
      ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();
  }
  ImGui::PopFont();
}

void Window::DebuggerWindow(n64::Core& core) const {
  ImGui::PushFont(uiFont);
  ImGui::Begin("Debugger");
  if(ImGui::Button("Step")) {
    core.debuggerState.gdb->config.step(&core);
  }
  ImGui::End();
  ImGui::PopFont();
}