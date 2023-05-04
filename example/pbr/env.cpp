#include "env.h"
#include "image.h"
#include "shaders/brdf_fs.h"
#include "shaders/brdf_vs.h"
#include "shaders/cubemap_vs.h"
#include "shaders/equirectangular_to_cubemap_fs.h"
#include "shaders/irradiance_convolution_fs.h"
#include "shaders/prefilter_fs.h"
#include <grapho/gl3/cubemapper.h>
#include <grapho/gl3/fbo.h>
#include <grapho/vertexlayout.h>
#include <iostream>
#include <learnopengl/filesystem.h>
#include <stb_image.h>

// pbr: load the HDR environment map
static uint32_t
LoadHdrTexture()
{
  Image image;
  if (!image.LoadHdr(
        FileSystem::getPath("resources/textures/hdr/newport_loft.hdr")
          .c_str())) {
    std::cout << "Failed to load HDR image." << std::endl;
    return {};
  }

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

  return hdrTexture;
}

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
  renderQuad();
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  return brdfLUTTexture;
}

// pbr: setup cubemap to render to and attach to framebuffer
static uint32_t
GenerateEnvCubeMap(const grapho::gl3::CubeMapper& mapper)
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

  mapper.Map(
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
GenerateIrradianceMap(const grapho::gl3::CubeMapper& mapper,
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

  mapper.Map(32,
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
GeneratePrefilterMap(const grapho::gl3::CubeMapper& mapper, uint32_t envCubemap)
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

    mapper.Map(
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

Environment::Environment()
{
  auto hdrTexture = LoadHdrTexture();
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, hdrTexture);

  grapho::gl3::CubeMapper mapper;
  envCubemap = GenerateEnvCubeMap(mapper);
  irradianceMap = GenerateIrradianceMap(mapper, envCubemap);
  prefilterMap = GeneratePrefilterMap(mapper, envCubemap);
  brdfLUTTexture = GenerateBrdfLUTTexture();
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
