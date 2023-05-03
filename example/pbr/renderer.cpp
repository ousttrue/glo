#include "renderer.h"
#include "ibl_specular_textured.h"
#include "mesh.h"
#include "pbrmaterial.h"
#include <glm/gtc/matrix_transform.hpp>
#include <learnopengl/filesystem.h>
#include <stb_image.h>
#include <stdexcept>

// utility function for loading a 2D texture from file
// ---------------------------------------------------
unsigned int
loadTexture(std::string_view path)
{
  unsigned int textureID;
  glGenTextures(1, &textureID);

  int width, height, nrComponents;
  std::string p(path.begin(), path.end());
  unsigned char* data = stbi_load(p.c_str(), &width, &height, &nrComponents, 0);
  if (data) {
    GLenum format;
    if (nrComponents == 1)
      format = GL_RED;
    else if (nrComponents == 3)
      format = GL_RGB;
    else if (nrComponents == 4)
      format = GL_RGBA;

    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 format,
                 width,
                 height,
                 0,
                 format,
                 GL_UNSIGNED_BYTE,
                 data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(
      GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
  } else {
    std::cout << "Texture failed to load at path: " << path << std::endl;
    stbi_image_free(data);
  }

  return textureID;
}

Renderer::Renderer()
{
  {
    auto pbrShader = grapho::gl3::ShaderProgram::CreateFromPath("2.2.2.pbr.vs",
                                                                "2.2.2.pbr.fs");
    if (!pbrShader) {
      throw std::runtime_error(pbrShader.error());
    }
    PbrShader = *pbrShader;
  }
  {
    auto backgroundShader = grapho::gl3::ShaderProgram::CreateFromPath(
      "2.2.2.background.vs", "2.2.2.background.fs");
    if (!backgroundShader) {
      throw std::runtime_error(backgroundShader.error());
    }
    BackgroundShader = *backgroundShader;
  }

  {
    auto sphere = Mesh::Sphere();

    auto vbo = grapho::gl3::Vbo::Create(sphere->Vertices);
    std::shared_ptr<grapho::gl3::Vbo> slots[]{
      vbo,
    };
    auto ibo = grapho::gl3::Ibo::Create(sphere->Indices);

    Sphere = grapho::gl3::Vao::Create(sphere->Layouts, slots, ibo);
    SphereDrawCount = sphere->Indices.size();
  }

  // configure global opengl state
  // -----------------------------
  glEnable(GL_DEPTH_TEST);
  // set depth function to less than AND equal for skybox depth trick.
  glDepthFunc(GL_LEQUAL);
  // enable seamless cubemap sampling for lower mip levels in the pre-filter
  // map.
  glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

  // build and compile shaders
  // -------------------------

  Shader equirectangularToCubemapShader("2.2.2.cubemap.vs",
                                        "2.2.2.equirectangular_to_cubemap.fs");
  Shader irradianceShader("2.2.2.cubemap.vs",
                          "2.2.2.irradiance_convolution.fs");
  Shader prefilterShader("2.2.2.cubemap.vs", "2.2.2.prefilter.fs");
  Shader brdfShader("2.2.2.brdf.vs", "2.2.2.brdf.fs");

  PbrShader->Use();
  PbrShader->Uniform("irradianceMap")->SetInt(0);
  PbrShader->Uniform("prefilterMap")->SetInt(1);
  PbrShader->Uniform("brdfLUT")->SetInt(2);
  PbrShader->Uniform("albedoMap")->SetInt(3);
  PbrShader->Uniform("normalMap")->SetInt(4);
  PbrShader->Uniform("metallicMap")->SetInt(5);
  PbrShader->Uniform("roughnessMap")->SetInt(6);
  PbrShader->Uniform("aoMap")->SetInt(7);

  BackgroundShader->Use();
  BackgroundShader->Uniform("environmentMap")->SetInt(0);

  // load PBR material textures
  // --------------------------
  // rusted iron
  Iron = PbrMaterial::Create(
    FileSystem::getPath("resources/textures/pbr/rusted_iron/albedo.png"),
    FileSystem::getPath("resources/textures/pbr/rusted_iron/normal.png"),
    FileSystem::getPath("resources/textures/pbr/rusted_iron/metallic.png"),
    FileSystem::getPath("resources/textures/pbr/rusted_iron/roughness.png"),
    FileSystem::getPath("resources/textures/pbr/rusted_iron/ao.png"));

  // gold
  goldAlbedoMap = loadTexture(
    FileSystem::getPath("resources/textures/pbr/gold/albedo.png").c_str());
  goldNormalMap = loadTexture(
    FileSystem::getPath("resources/textures/pbr/gold/normal.png").c_str());
  goldMetallicMap = loadTexture(
    FileSystem::getPath("resources/textures/pbr/gold/metallic.png").c_str());
  goldRoughnessMap = loadTexture(
    FileSystem::getPath("resources/textures/pbr/gold/roughness.png").c_str());
  goldAOMap = loadTexture(
    FileSystem::getPath("resources/textures/pbr/gold/ao.png").c_str());

  // grass
  grassAlbedoMap = loadTexture(
    FileSystem::getPath("resources/textures/pbr/grass/albedo.png").c_str());
  grassNormalMap = loadTexture(
    FileSystem::getPath("resources/textures/pbr/grass/normal.png").c_str());
  grassMetallicMap = loadTexture(
    FileSystem::getPath("resources/textures/pbr/grass/metallic.png").c_str());
  grassRoughnessMap = loadTexture(
    FileSystem::getPath("resources/textures/pbr/grass/roughness.png").c_str());
  grassAOMap = loadTexture(
    FileSystem::getPath("resources/textures/pbr/grass/ao.png").c_str());

  // plastic
  plasticAlbedoMap = loadTexture(
    FileSystem::getPath("resources/textures/pbr/plastic/albedo.png").c_str());
  plasticNormalMap = loadTexture(
    FileSystem::getPath("resources/textures/pbr/plastic/normal.png").c_str());
  plasticMetallicMap = loadTexture(
    FileSystem::getPath("resources/textures/pbr/plastic/metallic.png").c_str());
  plasticRoughnessMap = loadTexture(
    FileSystem::getPath("resources/textures/pbr/plastic/roughness.png")
      .c_str());
  plasticAOMap = loadTexture(
    FileSystem::getPath("resources/textures/pbr/plastic/ao.png").c_str());

  // wall
  wallAlbedoMap = loadTexture(
    FileSystem::getPath("resources/textures/pbr/wall/albedo.png").c_str());
  wallNormalMap = loadTexture(
    FileSystem::getPath("resources/textures/pbr/wall/normal.png").c_str());
  wallMetallicMap = loadTexture(
    FileSystem::getPath("resources/textures/pbr/wall/metallic.png").c_str());
  wallRoughnessMap = loadTexture(
    FileSystem::getPath("resources/textures/pbr/wall/roughness.png").c_str());
  wallAOMap = loadTexture(
    FileSystem::getPath("resources/textures/pbr/wall/ao.png").c_str());

  // pbr: setup framebuffer
  // ----------------------
  unsigned int captureFBO;
  unsigned int captureRBO;
  glGenFramebuffers(1, &captureFBO);
  glGenRenderbuffers(1, &captureRBO);

  glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
  glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
  glFramebufferRenderbuffer(
    GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

  // pbr: load the HDR environment map
  // ---------------------------------
  stbi_set_flip_vertically_on_load(true);
  int width, height, nrComponents;
  float* data = stbi_loadf(
    FileSystem::getPath("resources/textures/hdr/newport_loft.hdr").c_str(),
    &width,
    &height,
    &nrComponents,
    0);
  unsigned int hdrTexture;
  if (data) {
    glGenTextures(1, &hdrTexture);
    glBindTexture(GL_TEXTURE_2D, hdrTexture);
    glTexImage2D(
      GL_TEXTURE_2D,
      0,
      GL_RGB16F,
      width,
      height,
      0,
      GL_RGB,
      GL_FLOAT,
      data); // note how we specify the texture's data value to be float

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
  } else {
    std::cout << "Failed to load HDR image." << std::endl;
  }

  // pbr: setup cubemap to render to and attach to framebuffer
  // ---------------------------------------------------------
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

  // pbr: set up projection and view matrices for capturing data onto the 6
  // cubemap face directions
  // ----------------------------------------------------------------------------------------------
  glm::mat4 captureProjection =
    glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
  glm::mat4 captureViews[] = { glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f),
                                           glm::vec3(1.0f, 0.0f, 0.0f),
                                           glm::vec3(0.0f, -1.0f, 0.0f)),
                               glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f),
                                           glm::vec3(-1.0f, 0.0f, 0.0f),
                                           glm::vec3(0.0f, -1.0f, 0.0f)),
                               glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f),
                                           glm::vec3(0.0f, 1.0f, 0.0f),
                                           glm::vec3(0.0f, 0.0f, 1.0f)),
                               glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f),
                                           glm::vec3(0.0f, -1.0f, 0.0f),
                                           glm::vec3(0.0f, 0.0f, -1.0f)),
                               glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f),
                                           glm::vec3(0.0f, 0.0f, 1.0f),
                                           glm::vec3(0.0f, -1.0f, 0.0f)),
                               glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f),
                                           glm::vec3(0.0f, 0.0f, -1.0f),
                                           glm::vec3(0.0f, -1.0f, 0.0f)) };

  // pbr: convert HDR equirectangular environment map to cubemap equivalent
  // ----------------------------------------------------------------------
  equirectangularToCubemapShader.use();
  equirectangularToCubemapShader.setInt("equirectangularMap", 0);
  equirectangularToCubemapShader.setMat4("projection", captureProjection);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, hdrTexture);

  glViewport(
    0,
    0,
    512,
    512); // don't forget to configure the viewport to the capture dimensions.
  glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
  for (unsigned int i = 0; i < 6; ++i) {
    equirectangularToCubemapShader.setMat4("view", captureViews[i]);
    glFramebufferTexture2D(GL_FRAMEBUFFER,
                           GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                           envCubemap,
                           0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    renderCube();
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  // then let OpenGL generate mipmaps from first mip face (combatting visible
  // dots artifact)
  glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
  glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

  // pbr: create an irradiance cubemap, and re-scale capture FBO to irradiance
  // scale.
  // --------------------------------------------------------------------------------
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

  glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
  glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 32, 32);

  // pbr: solve diffuse integral by convolution to create an irradiance
  // (cube)map.
  // -----------------------------------------------------------------------------
  irradianceShader.use();
  irradianceShader.setInt("environmentMap", 0);
  irradianceShader.setMat4("projection", captureProjection);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);

  glViewport(
    0,
    0,
    32,
    32); // don't forget to configure the viewport to the capture dimensions.
  glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
  for (unsigned int i = 0; i < 6; ++i) {
    irradianceShader.setMat4("view", captureViews[i]);
    glFramebufferTexture2D(GL_FRAMEBUFFER,
                           GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                           irradianceMap,
                           0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    renderCube();
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  // pbr: create a pre-filter cubemap, and re-scale capture FBO to pre-filter
  // scale.
  // --------------------------------------------------------------------------------
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
  prefilterShader.use();
  prefilterShader.setInt("environmentMap", 0);
  prefilterShader.setMat4("projection", captureProjection);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);

  glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
  unsigned int maxMipLevels = 5;
  for (unsigned int mip = 0; mip < maxMipLevels; ++mip) {
    // reisze framebuffer according to mip-level size.
    unsigned int mipWidth = static_cast<unsigned int>(128 * std::pow(0.5, mip));
    unsigned int mipHeight =
      static_cast<unsigned int>(128 * std::pow(0.5, mip));
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(
      GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
    glViewport(0, 0, mipWidth, mipHeight);

    float roughness = (float)mip / (float)(maxMipLevels - 1);
    prefilterShader.setFloat("roughness", roughness);
    for (unsigned int i = 0; i < 6; ++i) {
      prefilterShader.setMat4("view", captureViews[i]);
      glFramebufferTexture2D(GL_FRAMEBUFFER,
                             GL_COLOR_ATTACHMENT0,
                             GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                             prefilterMap,
                             mip);

      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      renderCube();
    }
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  // pbr: generate a 2D LUT from the BRDF equations used.
  // ----------------------------------------------------
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
  glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
  glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
  glFramebufferTexture2D(
    GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brdfLUTTexture, 0);

  glViewport(0, 0, 512, 512);
  brdfShader.use();
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  renderQuad();

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

Renderer::~Renderer() {}

void
Renderer::Render(float currentFrame,
                 int scrWidth,
                 int scrHeight,
                 const DirectX::XMFLOAT4X4& projection,
                 const DirectX::XMFLOAT4X4& view,
                 const DirectX::XMFLOAT3& cameraPos)
{
  glViewport(0, 0, scrWidth, scrHeight);

  // per-frame time logic
  // --------------------
  deltaTime = currentFrame - lastFrame;
  lastFrame = currentFrame;

  // render
  // ------
  glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // render scene, supplying the convoluted irradiance map to the final
  // shader.
  // ------------------------------------------------------------------------------------------
  PbrShader->Use();
  glm::mat4 model = glm::mat4(1.0f);
  // auto view = cameraViewMatrix();
  PbrShader->Uniform("view")->SetMat4(view);
  // auto cameraPos = cameraPosition();
  PbrShader->Uniform("camPos")->SetFloat3(cameraPos);

  // initialize static shader uniforms before rendering
  // --------------------------------------------------
  // glm::mat4 projection = glm::perspective(glm::radians(cameraZoom()),
  //                                         (float)scrWidth / (float)scrHeight,
  //                                         0.1f,
  //                                         100.0f);
  PbrShader->Uniform("projection")->SetMat4(projection);

  // bind pre-computed IBL data
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);

  // rusted iron
  Iron->Bind();

  model = glm::mat4(1.0f);
  model = glm::translate(model, glm::vec3(-5.0, 0.0, 2.0));
  PbrShader->Uniform("model")->SetMat4(model);
  PbrShader->Uniform("normalMatrix")
    ->SetMat3(glm::transpose(glm::inverse(glm::mat3(model))));
  Sphere->Draw(GL_TRIANGLE_STRIP, SphereDrawCount);

  // gold
  glActiveTexture(GL_TEXTURE3);
  glBindTexture(GL_TEXTURE_2D, goldAlbedoMap);
  glActiveTexture(GL_TEXTURE4);
  glBindTexture(GL_TEXTURE_2D, goldNormalMap);
  glActiveTexture(GL_TEXTURE5);
  glBindTexture(GL_TEXTURE_2D, goldMetallicMap);
  glActiveTexture(GL_TEXTURE6);
  glBindTexture(GL_TEXTURE_2D, goldRoughnessMap);
  glActiveTexture(GL_TEXTURE7);
  glBindTexture(GL_TEXTURE_2D, goldAOMap);

  model = glm::mat4(1.0f);
  model = glm::translate(model, glm::vec3(-3.0, 0.0, 2.0));
  PbrShader->Uniform("model")->SetMat4(model);
  PbrShader->Uniform("normalMatrix")
    ->SetMat3(glm::transpose(glm::inverse(glm::mat3(model))));
  Sphere->Draw(GL_TRIANGLE_STRIP, SphereDrawCount);

  // grass
  glActiveTexture(GL_TEXTURE3);
  glBindTexture(GL_TEXTURE_2D, grassAlbedoMap);
  glActiveTexture(GL_TEXTURE4);
  glBindTexture(GL_TEXTURE_2D, grassNormalMap);
  glActiveTexture(GL_TEXTURE5);
  glBindTexture(GL_TEXTURE_2D, grassMetallicMap);
  glActiveTexture(GL_TEXTURE6);
  glBindTexture(GL_TEXTURE_2D, grassRoughnessMap);
  glActiveTexture(GL_TEXTURE7);
  glBindTexture(GL_TEXTURE_2D, grassAOMap);

  model = glm::mat4(1.0f);
  model = glm::translate(model, glm::vec3(-1.0, 0.0, 2.0));
  PbrShader->Uniform("model")->SetMat4(model);
  PbrShader->Uniform("normalMatrix")
    ->SetMat3(glm::transpose(glm::inverse(glm::mat3(model))));
  Sphere->Draw(GL_TRIANGLE_STRIP, SphereDrawCount);

  // plastic
  glActiveTexture(GL_TEXTURE3);
  glBindTexture(GL_TEXTURE_2D, plasticAlbedoMap);
  glActiveTexture(GL_TEXTURE4);
  glBindTexture(GL_TEXTURE_2D, plasticNormalMap);
  glActiveTexture(GL_TEXTURE5);
  glBindTexture(GL_TEXTURE_2D, plasticMetallicMap);
  glActiveTexture(GL_TEXTURE6);
  glBindTexture(GL_TEXTURE_2D, plasticRoughnessMap);
  glActiveTexture(GL_TEXTURE7);
  glBindTexture(GL_TEXTURE_2D, plasticAOMap);

  model = glm::mat4(1.0f);
  model = glm::translate(model, glm::vec3(1.0, 0.0, 2.0));
  PbrShader->Uniform("model")->SetMat4(model);
  PbrShader->Uniform("normalMatrix")
    ->SetMat3(glm::transpose(glm::inverse(glm::mat3(model))));
  Sphere->Draw(GL_TRIANGLE_STRIP, SphereDrawCount);

  // wall
  glActiveTexture(GL_TEXTURE3);
  glBindTexture(GL_TEXTURE_2D, wallAlbedoMap);
  glActiveTexture(GL_TEXTURE4);
  glBindTexture(GL_TEXTURE_2D, wallNormalMap);
  glActiveTexture(GL_TEXTURE5);
  glBindTexture(GL_TEXTURE_2D, wallMetallicMap);
  glActiveTexture(GL_TEXTURE6);
  glBindTexture(GL_TEXTURE_2D, wallRoughnessMap);
  glActiveTexture(GL_TEXTURE7);
  glBindTexture(GL_TEXTURE_2D, wallAOMap);

  model = glm::mat4(1.0f);
  model = glm::translate(model, glm::vec3(3.0, 0.0, 2.0));
  PbrShader->Uniform("model")->SetMat4(model);
  PbrShader->Uniform("normalMatrix")
    ->SetMat3(glm::transpose(glm::inverse(glm::mat3(model))));
  Sphere->Draw(GL_TRIANGLE_STRIP, SphereDrawCount);

  // render light source (simply re-render sphere at light positions)
  // this looks a bit off as we use the same shader, but it'll make their
  // positions obvious and keeps the codeprint small.
  for (unsigned int i = 0;
       i < sizeof(lightPositions) / sizeof(lightPositions[0]);
       ++i) {
    glm::vec3 newPos =
      lightPositions[i] + glm::vec3(sin(currentFrame * 5.0) * 5.0, 0.0, 0.0);
    newPos = lightPositions[i];
    PbrShader->Uniform("lightPositions[" + std::to_string(i) + "]")
      ->SetFloat3(newPos);
    PbrShader->Uniform("lightColors[" + std::to_string(i) + "]")
      ->SetFloat3(lightColors[i]);

    model = glm::mat4(1.0f);
    model = glm::translate(model, newPos);
    model = glm::scale(model, glm::vec3(0.5f));
    PbrShader->Uniform("model")->SetMat4(model);
    PbrShader->Uniform("normalMatrix")
      ->SetMat3(glm::transpose(glm::inverse(glm::mat3(model))));
    Sphere->Draw(GL_TRIANGLE_STRIP, SphereDrawCount);
  }

  // render skybox (render as last to prevent overdraw)
  BackgroundShader->Use();
  BackgroundShader->Uniform("projection")->SetMat4(projection);
  BackgroundShader->Uniform("view")->SetMat4(view);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
  // glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap); // display irradiance
  // map glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap); // display
  // prefilter map
  renderCube();

  // render BRDF map to screen
  // brdfShader.Use();
  // renderQuad();
}
