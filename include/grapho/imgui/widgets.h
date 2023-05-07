#pragma once
#include <imgui.h>
#include <imgui_internal.h>

namespace grapho {
namespace imgui {

inline struct
{
  bool IsActive;
  bool IsHovered;
} DraggableImage(ImTextureID texture, float w, float h)
{
  // image button. capture mouse event
  ImGui::ImageButton(
    texture, { w, h }, { 0, 1 }, { 1, 0 }, 0, { 1, 1, 1, 1 }, { 1, 1, 1, 1 });
  ImGui::ButtonBehavior(ImGui::GetCurrentContext()->LastItemData.Rect,
                        ImGui::GetCurrentContext()->LastItemData.ID,
                        nullptr,
                        nullptr,
                        ImGuiButtonFlags_MouseButtonMiddle |
                          ImGuiButtonFlags_MouseButtonRight);

  return {
    ImGui::IsItemActive(),
    ImGui::IsItemHovered(),
  };
}

}
}
