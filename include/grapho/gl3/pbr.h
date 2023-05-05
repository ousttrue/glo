#pragma once
#include "fbo.h"
#include "shader.h"
#include "shaders/brdf_fs.h"
#include "shaders/brdf_vs.h"
#include "shaders/cubemap_vs.h"
#include "shaders/equirectangular_to_cubemap_fs.h"
#include "shaders/irradiance_convolution_fs.h"
#include "shaders/prefilter_fs.h"
#include "texture.h"

namespace grapho {
namespace gl3 {

// pbr: generate a 2D LUT(look up table) from the BRDF equations used.
inline std::shared_ptr<grapho::gl3::Texture>
GenerateBrdfLUTTexture()
{
  auto brdfLUTTexture = grapho::gl3::Texture::Create(
    512, 512, grapho::PixelFormat::f16_RGB, nullptr, true);

  // then re-configure capture framebuffer object and render screen-space quad
  // with BRDF shader.
  grapho::gl3::Fbo fbo;
  fbo.AttachTexture2D(brdfLUTTexture->texture_);
  grapho::gl3::ClearViewport(grapho::Viewport{ 512, 512 });
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

// pbr: convert HDR equirectangular environment map to cubemap equivalent
inline void
GenerateEnvCubeMap(const grapho::gl3::CubeRenderer& cubeRenderer,
                   uint32_t envCubemap)
{
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
}

// pbr: solve diffuse integral by convolution to create an irradiance
// (cube)map.
inline void
GenerateIrradianceMap(const grapho::gl3::CubeRenderer& cubeRenderer,
                      uint32_t irradianceMap)
{
  auto irradianceShader =
    *grapho::gl3::ShaderProgram::Create(CUBEMAP_VS, IRRADIANCE_CONVOLUTION_FS);
  irradianceShader->Use();
  irradianceShader->Uniform("environmentMap")->SetInt(0);

  cubeRenderer.Render(
    32,
    irradianceMap,
    [irradianceShader](const auto& projection, const auto& view) {
      irradianceShader->Uniform("projection")->SetMat4(projection);
      irradianceShader->Uniform("view")->SetMat4(view);
    });
}

// pbr: run a quasi monte-carlo simulation on the environment lighting to
// create a prefilter (cube)map.
inline void
GeneratePrefilterMap(const grapho::gl3::CubeRenderer& cubeRenderer,
                     uint32_t prefilterMap)
{
  auto prefilterShader =
    *grapho::gl3::ShaderProgram::Create(CUBEMAP_VS, PREFILTER_FS);
  prefilterShader->Use();
  prefilterShader->Uniform("environmentMap")->SetInt(0);

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
}

}
}
