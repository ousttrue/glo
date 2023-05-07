#pragma once
#include <imgui.h>
#include <imgui_internal.h>

namespace grapho {
namespace imgui {

struct CursorState
{
  bool IsActive;
  bool IsHovered;
};

inline CursorState
DraggableImage(ImTextureID texture, const ImVec2& size)
{
  // image button. capture mouse event
  ImGui::ImageButton(
    texture, size, { 0, 1 }, { 1, 0 }, 0, { 1, 1, 1, 1 }, { 1, 1, 1, 1 });
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
