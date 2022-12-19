#pragma once
#include <imgui.h>

inline bool CreateComboList(const char* label, int* index, const char** items, int items_count) {
  if (ImGui::BeginCombo(label, items[*index])) {
    for (int n = 0; n < items_count; n++) {
      const bool is_selected = (*index == n);
      if (ImGui::Selectable(items[n], is_selected)) {
        *index = n;
      }

      if (is_selected) {
        ImGui::SetItemDefaultFocus();
      }
    }
    ImGui::EndCombo();
    return true;
  }

  return false;
}

#define checkjsonentry(name, type, param1, param2, defaultVal) \
  do { \
    auto name##Entry = settings[param1][param2];  \
    if(!name##Entry.empty()) {                    \
      auto value = name##Entry.get<type>();       \
      name = value;                               \
    } else {                                      \
      settingsFile.clear();                       \
      settings[param1][param2] = defaultVal;   \
      settingsFile << settings;                   \
      name = defaultVal;                          \
    }                                             \
  } while(0)
}