#pragma once
#include <imgui.h>

FORCE_INLINE bool CreateComboList(const char* label, int* index, const char** items, int items_count) {
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