#include "normalmap.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <filesystem>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <learnopengl/shader.h>

int
main(int argc, char** argv)
{
  if (argc < 2) {
    return 1;
  }
  std::filesystem::path dir = argv[1];

  // glfw: initialize and configure
  // ------------------------------
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
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
    return 2;
  }
  glfwMakeContextCurrent(window);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  glfwSetCursorPosCallback(window, mouse_callback);
  glfwSetScrollCallback(window, scroll_callback);

  // tell GLFW to capture our mouse
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  // glad: load all OpenGL function pointers
  // ---------------------------------------
  if (glewInit() != GLEW_OK) {
    std::cout << "Failed to initialize GLAD" << std::endl;
    return 3;
  }

  // configure global opengl state
  // -----------------------------
  glEnable(GL_DEPTH_TEST);

  // build and compile shaders
  // -------------------------
  Shader shader("4.normal_mapping.vs", "4.normal_mapping.fs");

  // load textures
  // -------------
  unsigned int diffuseMap =
    loadTexture((dir / "resources/textures/brickwall.jpg").string().c_str());
  unsigned int normalMap = loadTexture(
    (dir / "resources/textures/brickwall_normal.jpg").string().c_str());

  // shader configuration
  // --------------------
  shader.use();
  shader.setInt("diffuseMap", 0);
  shader.setInt("normalMap", 1);

  // lighting info
  // -------------
  glm::vec3 lightPos(0.5f, 1.0f, 0.3f);

  // render loop
  // -----------
  while (!glfwWindowShouldClose(window)) {
    // per-frame time logic
    // --------------------
    float currentFrame = static_cast<float>(glfwGetTime());
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    // input
    // -----
    processInput(window);

    // render
    // ------
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // configure view/projection matrices
    glm::mat4 projection =
      glm::perspective(glm::radians(camera.Zoom),
                       (float)SCR_WIDTH / (float)SCR_HEIGHT,
                       0.1f,
                       100.0f);
    glm::mat4 view = camera.GetViewMatrix();
    shader.use();
    shader.setMat4("projection", projection);
    shader.setMat4("view", view);
    // render normal-mapped quad
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::rotate(model,
                        glm::radians((float)glfwGetTime() * -10.0f),
                        glm::normalize(glm::vec3(
                          1.0, 0.0, 1.0))); // rotate the quad to show normal
                                            // mapping from multiple directions
    shader.setMat4("model", model);
    shader.setVec3("viewPos", camera.Position);
    shader.setVec3("lightPos", lightPos);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, diffuseMap);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, normalMap);
    renderQuad();

    // render light source (simply re-renders a smaller plane at the light's
    // position for debugging/visualization)
    model = glm::mat4(1.0f);
    model = glm::translate(model, lightPos);
    model = glm::scale(model, glm::vec3(0.1f));
    shader.setMat4("model", model);
    renderQuad();

    // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved
    // etc.)
    // -------------------------------------------------------------------------------
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwTerminate();
  return 0;
}
