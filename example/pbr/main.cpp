#include <GL/glew.h>

#include "drawable.h"
#include "image.h"
#include "pbrmaterial.h"
#include "shaders/brdf_fs.h"
#include "shaders/brdf_vs.h"
#include "shaders/cubemap_vs.h"
#include "shaders/equirectangular_to_cubemap_fs.h"
#include "shaders/irradiance_convolution_fs.h"
#include "shaders/prefilter_fs.h"
#include "skybox.h"

#include <GLFW/glfw3.h>
#include <grapho/gl3/cuberenderer.h>
#include <grapho/gl3/shader.h>
#include <grapho/orbitview.h>
#include <iostream>
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

// pbr: generate a 2D LUT(look up table) from the BRDF equations used.
static uint32_t
GenerateBrdfLUTTexture()
{
  uint32_t brdfLUTTexture;
  glGenTextures(1, &brdfLUTTexture);

  // pre-allocate enough memory for the LUT texture.
  glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, 512, 512, 0, GL_RG, GL_FLOAT, 0);
  // be sure to set wrapping mode to GL_CLAMP_TO_EDGE
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  // then re-configure capture framebuffer object and render screen-space quad
  // with BRDF shader.
  auto fbo = std::make_shared<grapho::gl3::Fbo>();
  grapho::Viewport fboViewport{ 512, 512 };
  fbo->AttachTexture2D(brdfLUTTexture);
  grapho::gl3::ClearViewport(fboViewport);
  auto brdfShader = *grapho::gl3::ShaderProgram::Create(BRDF_VS, BRDF_FS);
  brdfShader->Use();

  // renderQuad() renders a 1x1 XY quad in NDC
  auto quad = grapho::mesh::Quad();
  auto quadVao = grapho::gl3::Vao::Create(quad);
  auto drawCount = quad->DrawCount();
  auto drawMode = *grapho::gl3::GLMode(quad->Mode);
  quadVao->Draw(drawMode, drawCount);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  return brdfLUTTexture;
}

// pbr: setup cubemap to render to and attach to framebuffer
static uint32_t
GenerateEnvCubeMap(const grapho::gl3::CubeRenderer& cubeRenderer)
{
  uint32_t envCubemap;
  glGenTextures(1, &envCubemap);
  glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
  for (unsigned int i = 0; i < 6; ++i) {
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                 0,
                 GL_RGB16F,
                 512,
                 512,
                 0,
                 GL_RGB,
                 GL_FLOAT,
                 nullptr);
  }
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  glTexParameteri(
    GL_TEXTURE_CUBE_MAP,
    GL_TEXTURE_MIN_FILTER,
    GL_LINEAR_MIPMAP_LINEAR); // enable pre-filter mipmap sampling (combatting
                              // visible dots artifact)
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  // pbr: convert HDR equirectangular environment map to cubemap equivalent
  // ----------------------------------------------------------------------
  auto equirectangularToCubemapShader =
    *grapho::gl3::ShaderProgram::Create(CUBEMAP_VS, EQUIRECTANGULAR_FS);
  equirectangularToCubemapShader->Use();
  equirectangularToCubemapShader->Uniform("equirectangularMap")->SetInt(0);

  cubeRenderer.Render(
    512,
    envCubemap,
    [equirectangularToCubemapShader](const auto& projection, const auto& view) {
      equirectangularToCubemapShader->Uniform("projection")
        ->SetMat4(projection);
      equirectangularToCubemapShader->Uniform("view")->SetMat4(view);
    });

  // then let OpenGL generate mipmaps from first mip face (combatting visible
  // dots artifact)
  glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
  glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

  return envCubemap;
}

// pbr: create an irradiance cubemap, and re-scale capture FBO to irradiance
// scale.
static uint32_t
GenerateIrradianceMap(const grapho::gl3::CubeRenderer& cubeRenderer,
                      uint32_t envCubemap)
{
  uint32_t irradianceMap;
  glGenTextures(1, &irradianceMap);
  glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
  for (unsigned int i = 0; i < 6; ++i) {
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                 0,
                 GL_RGB16F,
                 32,
                 32,
                 0,
                 GL_RGB,
                 GL_FLOAT,
                 nullptr);
  }
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  // pbr: solve diffuse integral by convolution to create an irradiance
  // (cube)map.
  // -----------------------------------------------------------------------------
  auto irradianceShader =
    *grapho::gl3::ShaderProgram::Create(CUBEMAP_VS, IRRADIANCE_CONVOLUTION_FS);
  irradianceShader->Use();
  irradianceShader->Uniform("environmentMap")->SetInt(0);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);

  cubeRenderer.Render(32,
             irradianceMap,
             [irradianceShader](const auto& projection, const auto& view) {
               irradianceShader->Uniform("projection")->SetMat4(projection);
               irradianceShader->Uniform("view")->SetMat4(view);
             });

  return irradianceMap;
}

// pbr: create a pre-filter cubemap, and re-scale capture FBO to pre-filter
// scale.
static uint32_t
GeneratePrefilterMap(const grapho::gl3::CubeRenderer& cubeRenderer, uint32_t envCubemap)
{
  uint32_t prefilterMap;
  glGenTextures(1, &prefilterMap);
  glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
  for (unsigned int i = 0; i < 6; ++i) {
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                 0,
                 GL_RGB16F,
                 128,
                 128,
                 0,
                 GL_RGB,
                 GL_FLOAT,
                 nullptr);
  }
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP,
                  GL_TEXTURE_MIN_FILTER,
                  GL_LINEAR_MIPMAP_LINEAR); // be sure to set minification
                                            // filter to mip_linear
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  // generate mipmaps for the cubemap so OpenGL automatically allocates the
  // required memory.
  glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

  // pbr: run a quasi monte-carlo simulation on the environment lighting to
  // create a prefilter (cube)map.
  // ----------------------------------------------------------------------------------------------------
  auto prefilterShader =
    *grapho::gl3::ShaderProgram::Create(CUBEMAP_VS, PREFILTER_FS);
  prefilterShader->Use();
  prefilterShader->Uniform("environmentMap")->SetInt(0);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);

  unsigned int maxMipLevels = 5;
  for (unsigned int mip = 0; mip < maxMipLevels; ++mip) {
    // reisze framebuffer according to mip-level size.
    auto mipSize = static_cast<int>(128 * std::pow(0.5, mip));
    float roughness = (float)mip / (float)(maxMipLevels - 1);
    prefilterShader->Uniform("roughness")->SetFloat(roughness);

    cubeRenderer.Render(
      mipSize,
      prefilterMap,
      [prefilterShader](const auto& projection, const auto& view) {
        prefilterShader->Uniform("projection")->SetMat4(projection);
        prefilterShader->Uniform("view")->SetMat4(view);
      },
      mip);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }

  return prefilterMap;
}

struct Scene
{
  std::vector<std::shared_ptr<Drawable>> Drawables;

  void Load(const std::filesystem::path& baseDir)
  {
    auto iron = std::make_shared<Drawable>();
    iron->Material = PbrMaterial::Create(
      baseDir / "resources/textures/pbr/rusted_iron/albedo.png",
      baseDir / "resources/textures/pbr/rusted_iron/normal.png",
      baseDir / "resources/textures/pbr/rusted_iron/metallic.png",
      baseDir / "resources/textures/pbr/rusted_iron/roughness.png",
      baseDir / "resources/textures/pbr/rusted_iron/ao.png");
    iron->Position = { -5.0, 0.0, 2.0 };
    Drawables.push_back(iron);

    auto gold = std::make_shared<Drawable>();
    gold->Material = PbrMaterial::Create(
      baseDir / ("resources/textures/pbr/gold/albedo.png"),
      baseDir / ("resources/textures/pbr/gold/normal.png"),
      baseDir / ("resources/textures/pbr/gold/metallic.png"),
      baseDir / ("resources/textures/pbr/gold/roughness.png"),
      baseDir / ("resources/textures/pbr/gold/ao.png"));
    gold->Position = { -3.0, 0.0, 2.0 };
    Drawables.push_back(gold);

    auto grass = std::make_shared<Drawable>();
    grass->Material = PbrMaterial::Create(
      baseDir / ("resources/textures/pbr/grass/albedo.png"),
      baseDir / ("resources/textures/pbr/grass/normal.png"),
      baseDir / ("resources/textures/pbr/grass/metallic.png"),
      baseDir / ("resources/textures/pbr/grass/roughness.png"),
      baseDir / ("resources/textures/pbr/grass/ao.png"));
    grass->Position = { -1.0, 0.0, 2.0 };
    Drawables.push_back(grass);

    auto plastic = std::make_shared<Drawable>();
    plastic->Material = PbrMaterial::Create(
      baseDir / ("resources/textures/pbr/plastic/albedo.png"),
      baseDir / ("resources/textures/pbr/plastic/normal.png"),
      baseDir / ("resources/textures/pbr/plastic/metallic.png"),
      baseDir / ("resources/textures/pbr/plastic/roughness.png"),
      baseDir / ("resources/textures/pbr/plastic/ao.png"));
    plastic->Position = { 1.0, 0.0, 2.0 };
    Drawables.push_back(plastic);

    auto wall = std::make_shared<Drawable>();
    wall->Material = PbrMaterial::Create(
      baseDir / ("resources/textures/pbr/wall/albedo.png"),
      baseDir / ("resources/textures/pbr/wall/normal.png"),
      baseDir / ("resources/textures/pbr/wall/metallic.png"),
      baseDir / ("resources/textures/pbr/wall/roughness.png"),
      baseDir / ("resources/textures/pbr/wall/ao.png"));
    wall->Position = { 3.0, 0.0, 2.0 };
    Drawables.push_back(wall);
  }
};

int
main(int argc, char** argv)
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
    std::cout << "Failed to initialize GLEW" << std::endl;
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
  std::filesystem::path dir(argv[1]);
  Image image;
  if (!image.LoadHdr(dir / ("resources/textures/hdr/newport_loft.hdr"))) {
    std::cout << "Failed to load HDR image." << std::endl;
    return 3;
  }

  // HDR
  unsigned int hdrTexture;
  glGenTextures(1, &hdrTexture);
  glBindTexture(GL_TEXTURE_2D, hdrTexture);
  glTexImage2D(
    GL_TEXTURE_2D,
    0,
    image.Format,
    image.Width,
    image.Height,
    0,
    GL_RGB,
    GL_FLOAT,
    image.Data); // note how we specify the texture's data value to be float

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, hdrTexture);

  grapho::gl3::CubeRenderer cubeRenderer;
  auto envCubemap = GenerateEnvCubeMap(cubeRenderer);
  auto irradianceMap = GenerateIrradianceMap(cubeRenderer, envCubemap);
  auto prefilterMap = GeneratePrefilterMap(cubeRenderer, envCubemap);
  auto brdfLUTTexture = GenerateBrdfLUTTexture();

  Scene scene;
  scene.Load(dir);
  Lights lights{};
  Skybox skybox(envCubemap);

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
      // ENV
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
      glActiveTexture(GL_TEXTURE1);
      glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
      glActiveTexture(GL_TEXTURE2);
      glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);
      // OBJECTS
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
