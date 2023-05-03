#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

void
mouse_callback(struct GLFWwindow* window, double xposIn, double yposIn);
void
scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

float
cameraZoom();
glm::mat4
cameraViewMatrix();
glm::vec3
cameraPosition();
