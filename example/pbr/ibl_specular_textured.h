#pragma once
#include <GLFW/glfw3.h>

// settings
inline constexpr unsigned int SCR_WIDTH = 1280;
inline constexpr unsigned int SCR_HEIGHT = 720;

// timing
extern float deltaTime;
extern float lastFrame;

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
renderSphere();
void
renderCube();
void
renderQuad();

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

float
cameraZoom();
glm::mat4
cameraViewMatrix();
glm::vec3
cameraPosition();
