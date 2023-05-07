#pragma once
#include <functional>
#include <optional>

class GlfwPlatform
{
  struct GLFWwindow* m_window = nullptr;

public:
  GlfwPlatform();
  ~GlfwPlatform();
  GLFWwindow* CreateWindow(const char* titlle, int width, int height);
  struct Frame
  {
    int Width;
    int Height;
  };
  std::optional<Frame> BeginFrame();
  void EndFrame(const std::function<void()>& callback = {});
};
