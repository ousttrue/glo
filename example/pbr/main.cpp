#include <GL/glew.h>

#include "drawable.h"
#include "env.h"
#include "pbrmaterial.h"
#include "skybox.h"
#include <GLFW/glfw3.h>
#include <grapho/orbitview.h>
#include <iostream>
#include <learnopengl/filesystem.h>
#include <vector>

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

struct Scene
{
  std::vector<std::shared_ptr<Drawable>> Drawables;

  void Load()
  {
    auto iron = std::make_shared<Drawable>();
    iron->Material = PbrMaterial::Create(
      FileSystem::getPath("resources/textures/pbr/rusted_iron/albedo.png"),
      FileSystem::getPath("resources/textures/pbr/rusted_iron/normal.png"),
      FileSystem::getPath("resources/textures/pbr/rusted_iron/metallic.png"),
      FileSystem::getPath("resources/textures/pbr/rusted_iron/roughness.png"),
      FileSystem::getPath("resources/textures/pbr/rusted_iron/ao.png"));
    iron->Position = { -5.0, 0.0, 2.0 };
    Drawables.push_back(iron);

    auto gold = std::make_shared<Drawable>();
    gold->Material = PbrMaterial::Create(
      FileSystem::getPath("resources/textures/pbr/gold/albedo.png"),
      FileSystem::getPath("resources/textures/pbr/gold/normal.png"),
      FileSystem::getPath("resources/textures/pbr/gold/metallic.png"),
      FileSystem::getPath("resources/textures/pbr/gold/roughness.png"),
      FileSystem::getPath("resources/textures/pbr/gold/ao.png"));
    gold->Position = { -3.0, 0.0, 2.0 };
    Drawables.push_back(gold);

    auto grass = std::make_shared<Drawable>();
    grass->Material = PbrMaterial::Create(
      FileSystem::getPath("resources/textures/pbr/grass/albedo.png"),
      FileSystem::getPath("resources/textures/pbr/grass/normal.png"),
      FileSystem::getPath("resources/textures/pbr/grass/metallic.png"),
      FileSystem::getPath("resources/textures/pbr/grass/roughness.png"),
      FileSystem::getPath("resources/textures/pbr/grass/ao.png"));
    grass->Position = { -1.0, 0.0, 2.0 };
    Drawables.push_back(grass);

    auto plastic = std::make_shared<Drawable>();
    plastic->Material = PbrMaterial::Create(
      FileSystem::getPath("resources/textures/pbr/plastic/albedo.png"),
      FileSystem::getPath("resources/textures/pbr/plastic/normal.png"),
      FileSystem::getPath("resources/textures/pbr/plastic/metallic.png"),
      FileSystem::getPath("resources/textures/pbr/plastic/roughness.png"),
      FileSystem::getPath("resources/textures/pbr/plastic/ao.png"));
    plastic->Position = { 1.0, 0.0, 2.0 };
    Drawables.push_back(plastic);

    auto wall = std::make_shared<Drawable>();
    wall->Material = PbrMaterial::Create(
      FileSystem::getPath("resources/textures/pbr/wall/albedo.png"),
      FileSystem::getPath("resources/textures/pbr/wall/normal.png"),
      FileSystem::getPath("resources/textures/pbr/wall/metallic.png"),
      FileSystem::getPath("resources/textures/pbr/wall/roughness.png"),
      FileSystem::getPath("resources/textures/pbr/wall/ao.png"));
    wall->Position = { 3.0, 0.0, 2.0 };
    Drawables.push_back(wall);
  }
};

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

  // configure global opengl state
  // -----------------------------
  glEnable(GL_DEPTH_TEST);
  // set depth function to less than AND equal for skybox depth trick.
  glDepthFunc(GL_LEQUAL);
  // enable seamless cubemap sampling for lower mip levels in the pre-filter
  // map.
  glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

  //
  // scene
  //
  Environment Env;
  Scene scene;
  scene.Load();
  Lights lights{};
  Skybox skybox(Env.envCubemap);

  while (!glfwWindowShouldClose(window)) {
    //
    // update
    //
    glfwPollEvents();
    processInput(window);
    int scrWidth, scrHeight;
    glfwGetFramebufferSize(window, &scrWidth, &scrHeight);
    g_camera.SetSize(scrWidth, scrHeight);
    DirectX::XMFLOAT4X4 projection;
    DirectX::XMFLOAT4X4 view;
    g_camera.Update(&projection._11, &view._11);

    //
    // render
    //
    glViewport(0, 0, scrWidth, scrHeight);
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    {
      Env.Bind();
      for (auto& drawable : scene.Drawables) {
        drawable->Draw(projection, view, g_camera.Position, lights);
      }
      skybox.Draw(projection, view);
    }

    glfwSwapBuffers(window);
  }

  glfwTerminate();

  return 0;
}
