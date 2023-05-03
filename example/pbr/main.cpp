#include <GL/glew.h>

#include "renderer.h"
#include <GLFW/glfw3.h>
#include <grapho/orbitview.h>
#include <iostream>

// settings
const auto SCR_WIDTH = 1600;
const auto SCR_HEIGHT = 1200;

grapho::OrbitView g_camera;

// process all input: query GLFW whether relevant keys are pressed/released this
// frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
static void
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

bool g_mouseRight = false;
bool g_mouseMiddle = false;
static void
mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
  switch (button) {
    case GLFW_MOUSE_BUTTON_RIGHT:
      if (action == GLFW_PRESS) {
        g_mouseRight = true;
      } else if (action == GLFW_RELEASE) {
        g_mouseRight = false;
      }
      break;
    case GLFW_MOUSE_BUTTON_MIDDLE:
      if (action == GLFW_PRESS) {
        g_mouseMiddle = true;
      } else if (action == GLFW_RELEASE) {
        g_mouseMiddle = false;
      }
      break;
  }
}

static void
mouse_cursor_callback(GLFWwindow* window, double xposIn, double yposIn)
{
  static float lastX = 800.0f / 2.0;
  static float lastY = 600.0 / 2.0;
  static bool firstMouse = true;
  float xpos = static_cast<float>(xposIn);
  float ypos = static_cast<float>(yposIn);
  if (firstMouse) {
    firstMouse = false;
  } else {
    auto xoffset = static_cast<int>(xpos - lastX);
    auto yoffset = static_cast<int>(ypos - lastY);
    if (g_mouseRight) {
      g_camera.YawPitch(xoffset, yoffset);
    } else if (g_mouseMiddle) {
      g_camera.Shift(xoffset, yoffset);
    }
  }
  lastX = xpos;
  lastY = ypos;
}

static void
scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
  g_camera.Dolly(static_cast<int>(yoffset));
}

int
main()
{
  g_camera.shift_[2] = -10;

  // glfw: initialize and configure
  // ------------------------------
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_SAMPLES, 4);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

  // glfw window creation
  // --------------------
  GLFWwindow* window =
    glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
  if (window == NULL) {
    std::cout << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return 1;
  }

  glfwMakeContextCurrent(window);
  if (glewInit() != GLEW_OK) {
    std::cout << "Failed to initialize GLAD" << std::endl;
    return 2;
  }

  glfwSetMouseButtonCallback(window, mouse_button_callback);
  glfwSetCursorPosCallback(window, mouse_cursor_callback);
  glfwSetScrollCallback(window, scroll_callback);

  Renderer renderer;
  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
    processInput(window);

    // then before rendering, configure the viewport to the original
    // framebuffer's screen dimensions
    int scrWidth, scrHeight;
    glfwGetFramebufferSize(window, &scrWidth, &scrHeight);

    g_camera.SetSize(scrWidth, scrHeight);

    DirectX::XMFLOAT4X4 projection;
    DirectX::XMFLOAT4X4 view;
    g_camera.Update(&projection._11, &view._11);

    renderer.Render(scrWidth, scrHeight, projection, view, g_camera.Position);

    glfwSwapBuffers(window);
  }

  glfwTerminate();

  return 0;
}
