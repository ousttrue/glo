#include "glfw_platform.h"
#include <GLFW/glfw3.h>
#include <iostream>

// process all input: query GLFW whether relevant keys are pressed/released this
// frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void
processInput(GLFWwindow* window)
{
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);

  // if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
  //   camera.ProcessKeyboard(FORWARD, deltaTime);
  // if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
  //   camera.ProcessKeyboard(BACKWARD, deltaTime);
  // if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
  //   camera.ProcessKeyboard(LEFT, deltaTime);
  // if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
  //   camera.ProcessKeyboard(RIGHT, deltaTime);
}

GlfwPlatform::GlfwPlatform()
{
  glfwInit();
}

GlfwPlatform::~GlfwPlatform()
{
  if (m_window) {
    glfwDestroyWindow(m_window);
  }
  glfwTerminate();
}

GLFWwindow*
GlfwPlatform::CreateWindow(const char* title, int width, int height)
{
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

  m_window = glfwCreateWindow(width, height, title, nullptr, nullptr);
  if (!m_window) {
    std::cout << "Failed to create GLFW window" << std::endl;
    return nullptr;
  }

  glfwSetWindowUserPointer(m_window, this);
  glfwMakeContextCurrent(m_window);
  glfwSwapInterval(1); // Enable vsync

  return m_window;
}

bool
GlfwPlatform::BeginFrame()
{
  if (glfwWindowShouldClose(m_window)) {
    return false;
  }
  glfwPollEvents();
  processInput(m_window);
  int width, height;
  glfwGetFramebufferSize(m_window, &width, &height);

  return true;
}

void
GlfwPlatform::EndFrame(const std::function<void()>& callback)
{
  GLFWwindow* backup_current_context = glfwGetCurrentContext();
  if (callback) {
    callback();
  }
  glfwMakeContextCurrent(backup_current_context);

  glfwSwapBuffers(m_window);
}
