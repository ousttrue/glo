#include "camera.h"

#include <learnopengl/camera.h>

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
float lastX = 800.0f / 2.0;
float lastY = 600.0 / 2.0;
bool firstMouse = true;
void
mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
  float xpos = static_cast<float>(xposIn);
  float ypos = static_cast<float>(yposIn);

  if (firstMouse) {
    lastX = xpos;
    lastY = ypos;
    firstMouse = false;
  }

  float xoffset = xpos - lastX;
  float yoffset =
    lastY - ypos; // reversed since y-coordinates go from bottom to top

  lastX = xpos;
  lastY = ypos;

  camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void
scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
  camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

float
cameraZoom()
{
  return camera.Zoom;
}
glm::mat4
cameraViewMatrix()
{
  return camera.GetViewMatrix();
}
glm::vec3
cameraPosition()
{
  return camera.Position;
}
