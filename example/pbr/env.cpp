#include "env.h"
#include "mesh.h"
#include <glm/gtc/matrix_transform.hpp>
#include <grapho/gl3/fbo.h>
#include <learnopengl/filesystem.h>
#include <learnopengl/shader.h>
#include <stb_image.h>

// renderQuad() renders a 1x1 XY quad in NDC
// -----------------------------------------
static void
renderQuad()
{
  static unsigned int quadVAO = 0;
  static unsigned int quadVBO;
  if (quadVAO == 0) {
    float quadVertices[] = {
      // positions        // texture Coords
      -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
      1.0f,  1.0f, 0.0f, 1.0f, 1.0f, 1.0f,  -1.0f, 0.0f, 1.0f, 0.0f,
    };
    // setup plane VAO
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(
      GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
      0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
      1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
  }
  glBindVertexArray(quadVAO);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  glBindVertexArray(0);
}

Environment::Environment()
{
  {
    auto cube = Mesh::Cube();
    auto vbo = grapho::gl3::Vbo::Create(cube->Vertices);
    std::shared_ptr<grapho::gl3::Vbo> slots[]{ vbo };
    Cube = grapho::gl3::Vao::Create(cube->Layouts, slots);
    CubeDrawCount = cube->Vertices.size();
  }

  Shader equirectangularToCubemapShader("2.2.2.cubemap.vs",
                                        "2.2.2.equirectangular_to_cubemap.fs");
  Shader irradianceShader("2.2.2.cubemap.vs",
                          "2.2.2.irradiance_convolution.fs");
  Shader prefilterShader("2.2.2.cubemap.vs", "2.2.2.prefilter.fs");

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

  {
    auto fbo = std::make_shared<grapho::gl3::Fbo>();
    grapho::Viewport fboViewport{ 512, 512 };
    for (unsigned int i = 0; i < 6; ++i) {
      equirectangularToCubemapShader.setMat4("view", captureViews[i]);
      fbo->AttachCubeMap(i, envCubemap);
      grapho::gl3::ClearViewport(fboViewport);
      Cube->Draw(GL_TRIANGLES, CubeDrawCount);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }

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

  // pbr: solve diffuse integral by convolution to create an irradiance
  // (cube)map.
  // -----------------------------------------------------------------------------
  irradianceShader.use();
  irradianceShader.setInt("environmentMap", 0);
  irradianceShader.setMat4("projection", captureProjection);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);

  {
    auto fbo = std::make_shared<grapho::gl3::Fbo>();
    grapho::Viewport fboViewport{ 32, 32 };
    for (unsigned int i = 0; i < 6; ++i) {
      irradianceShader.setMat4("view", captureViews[i]);
      fbo->AttachCubeMap(i, irradianceMap);
      grapho::gl3::ClearViewport(fboViewport);
      Cube->Draw(GL_TRIANGLES, CubeDrawCount);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }

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

  unsigned int maxMipLevels = 5;
  for (unsigned int mip = 0; mip < maxMipLevels; ++mip) {
    // reisze framebuffer according to mip-level size.
    auto mipWidth = static_cast<int>(128 * std::pow(0.5, mip));
    auto mipHeight = static_cast<int>(128 * std::pow(0.5, mip));
    grapho::Viewport fboViewport{ mipWidth, mipHeight };

    float roughness = (float)mip / (float)(maxMipLevels - 1);
    prefilterShader.setFloat("roughness", roughness);

    auto fbo = std::make_shared<grapho::gl3::Fbo>();
    for (unsigned int i = 0; i < 6; ++i) {
      prefilterShader.setMat4("view", captureViews[i]);
      fbo->AttachCubeMap(i, prefilterMap, mip);
      grapho::gl3::ClearViewport(fboViewport);
      Cube->Draw(GL_TRIANGLES, CubeDrawCount);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }

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
  {
    auto fbo = std::make_shared<grapho::gl3::Fbo>();
    grapho::Viewport fboViewport{ 512, 512 };
    fbo->AttachTexture2D(brdfLUTTexture);
    grapho::gl3::ClearViewport(fboViewport);
    Shader brdfShader("2.2.2.brdf.vs", "2.2.2.brdf.fs");
    brdfShader.use();
    renderQuad();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }
}

void
Environment::Bind()
{
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);
}
