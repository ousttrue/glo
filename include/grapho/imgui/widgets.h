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

// https://github.com/ocornut/imgui/issues/319
inline bool
Splitter(bool split_vertically,
         float thickness,
         float* size1,
         float* size2,
         float min_size1,
         float min_size2,
         float splitter_long_axis_size = -1.0f)
{
  using namespace ImGui;
  ImGuiContext& g = *GImGui;
  ImGuiWindow* window = g.CurrentWindow;
  ImGuiID id = window->GetID("##Splitter");
  ImRect bb;
  bb.Min = window->DC.CursorPos +
           (split_vertically ? ImVec2(*size1, 0.0f) : ImVec2(0.0f, *size1));
  bb.Max = bb.Min + CalcItemSize(split_vertically
                                   ? ImVec2(thickness, splitter_long_axis_size)
                                   : ImVec2(splitter_long_axis_size, thickness),
                                 0.0f,
                                 0.0f);
  return SplitterBehavior(bb,
                          id,
                          split_vertically ? ImGuiAxis_X : ImGuiAxis_Y,
                          size1,
                          size2,
                          min_size1,
                          min_size2,
                          0.0f);
}

template<typename T, size_t N>
static void
EnumCombo(const char* label,
          T* value,
          const std::tuple<T, const char*> (&list)[N])
{
  using TUPLE = std::tuple<T, const char*>;
  int i = 0;
  for (; i < N; ++i) {
    if (std::get<0>(list[i]) == *value) {
      break;
    }
  }

  auto callback = [](void* data, int n, const char** out_str) -> bool {
    if (n < N) {
      *out_str = std::get<1>(((const TUPLE*)data)[n]);
      return true;
    }
    return false;
  };
  if (ImGui::Combo(label, &i, callback, (void*)list, N)) {
    *value = std::get<0>(list[i]);
  }
}

}
}
