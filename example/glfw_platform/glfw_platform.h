#pragma once
#include <functional>

class GlfwPlatform
{
  struct GLFWwindow* m_window = nullptr;

public:
  GlfwPlatform();
  ~GlfwPlatform();
  GLFWwindow* CreateWindow(const char* titlle, int width, int height);
  bool BeginFrame();
  void EndFrame(const std::function<void()>& callback);
};
