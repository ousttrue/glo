#pragma once
#include <functional>
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

class TreeSplitter
{
  struct UI
  {
    std::string Label;
    std::function<void()> Show;
    std::list<UI> Children;

    UI* ShowSelector(const UI* selected)
    {
      static ImGuiTreeNodeFlags base_flags =
        ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick |
        ImGuiTreeNodeFlags_SpanAvailWidth;

      ImGuiTreeNodeFlags node_flags = base_flags;
      if (this == selected) {
        node_flags |= ImGuiTreeNodeFlags_Selected;
      }
      if (Children.empty()) {
        node_flags |= ImGuiTreeNodeFlags_Leaf;
      }

      bool node_open =
        ImGui::TreeNodeEx((void*)this, node_flags, "%s", Label.c_str());

      UI* clicked = nullptr;
      if (node_open) {
        for (auto& child : Children) {
          if (child.ShowSelector(selected)) {
            clicked = &child;
          }
        }
        ImGui::TreePop();
      }

      if (clicked) {
        return clicked;
      } else if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
        return this;
      } else {
        return nullptr;
      }
    }
  };
  std::list<UI> m_list;
  UI* m_selected = nullptr;
  float m_splitter = 200.0f;

public:
  void Clear()
  {
    m_list.clear();
    m_selected = nullptr;
  }

  UI* Push(const char* label,
           UI* parent = nullptr,
           const std::function<void()>& callback = {})
  {
    auto& list = parent ? parent->Children : m_list;
    list.push_back({
      .Label = label,
      .Show = callback,
    });
    return &list.back();
  }

  void ShowGui()
  {
    // auto size = ImGui::GetCurrentWindow()->Size;
    auto size = ImGui::GetContentRegionAvail();
    float s = size.x - m_splitter - 5;
    // ImGui::Text("%f, %f: %f; %f", size.x, size.y, f, s);
    grapho::imgui::Splitter(true, 5, &m_splitter, &s, 8, 8);

    ShowGuiLeft(m_splitter);
    ShowGuiRight();
  }

private:
  void ShowGuiLeft(float w)
  {
    ImGui::BeginChild("left pane", ImVec2(w, 0), true);

    ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 0);
    UI* node_clicked = nullptr;
    for (auto& ui : m_list) {
      if (auto current = ui.ShowSelector(m_selected)) {
        node_clicked = current;
      }
    }
    if (node_clicked) {
      m_selected = node_clicked;
    }
    ImGui::PopStyleVar();

    ImGui::EndChild();
  }

  void ShowGuiRight()
  {
    ImGui::SameLine();

    // Leave room for 1 line below us
    ImGui::BeginChild("item view",
                      ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));
    if (m_selected && m_selected->Show) {
      m_selected->Show();
    }
    ImGui::EndChild();
  }
};

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
