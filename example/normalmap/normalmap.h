#pragma once
#include <learnopengl/camera.h>

struct GLFWwindow;

// settings
inline constexpr unsigned int SCR_WIDTH = 800;
inline constexpr unsigned int SCR_HEIGHT = 600;

extern float deltaTime;
extern float lastFrame;
extern Camera camera;

void
framebuffer_size_callback(GLFWwindow* window, int width, int height);
void
mouse_callback(GLFWwindow* window, double xpos, double ypos);
void
scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void
processInput(GLFWwindow* window);
unsigned int
loadTexture(const char* path);
void
renderQuad();
