#pragma once
#include "cuberenderer.h"
#include "fbo.h"
#include "shader.h"
#include "texture.h"
#include "ubo.h"
#include "vao.h"

namespace grapho {
namespace gl3 {

// pbr: generate a 2D LUT(look up table) from the BRDF equations used.
inline std::shared_ptr<grapho::gl3::Texture>
GenerateBrdfLUTTexture()
{
#include "shaders/brdf_fs.h"
#include "shaders/brdf_vs.h"

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
#include "shaders/cubemap_vs.h"
#include "shaders/equirectangular_to_cubemap_fs.h"

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
#include "shaders/cubemap_vs.h"
#include "shaders/irradiance_convolution_fs.h"
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
#include "shaders/cubemap_vs.h"
#include "shaders/prefilter_fs.h"
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

struct Lights
{
  DirectX::XMFLOAT4 Positions[4] = {
    { -10.0f, 10.0f, 10.0f, 0 },
    { 10.0f, 10.0f, 10.0f, 0 },
    { -10.0f, -10.0f, 10.0f, 0 },
    { 10.0f, -10.0f, 10.0f, 0 },
  };
  DirectX::XMFLOAT4 Colors[4] = {
    { 300.0f, 300.0f, 300.0f, 0 },
    { 300.0f, 300.0f, 300.0f, 0 },
    { 300.0f, 300.0f, 300.0f, 0 },
    { 300.0f, 300.0f, 300.0f, 0 },
  };
};

struct PbrEnv
{
  std::shared_ptr<Cubemap> EnvCubemap;
  std::shared_ptr<Cubemap> IrradianceMap;
  std::shared_ptr<Cubemap> PrefilterMap;
  std::shared_ptr<Texture> BrdfLUTTexture;

  // light ubo
  Lights lights{};
  std::shared_ptr<grapho::gl3::Ubo> LightsUbo;
  static const uint32_t UBO_LIGHTS_BINDING = 0;

  // Skybox skybox;
  std::shared_ptr<grapho::gl3::Vao> Cube;
  uint32_t CubeDrawCount = 0;
  std::shared_ptr<grapho::gl3::ShaderProgram> BackgroundShader;

  PbrEnv(const std::shared_ptr<Texture>& hdrTexture)
  {
    EnvCubemap = grapho::gl3::Cubemap::Create(
      512, 512, grapho::PixelFormat::f16_RGB, true);
    EnvCubemap->SamplingLinear(true);

    // hdr to cuemap
    hdrTexture->Activate(0);
    grapho::gl3::CubeRenderer cubeRenderer;
    grapho::gl3::GenerateEnvCubeMap(cubeRenderer, EnvCubemap->texture_);
    EnvCubemap->GenerateMipmap();
    EnvCubemap->UnBind();

    // irradianceMap
    IrradianceMap =
      grapho::gl3::Cubemap::Create(32, 32, grapho::PixelFormat::f16_RGB, true);
    EnvCubemap->Activate(0);
    grapho::gl3::GenerateIrradianceMap(cubeRenderer, IrradianceMap->texture_);

    // prefilterMap
    PrefilterMap = grapho::gl3::Cubemap::Create(
      128, 128, grapho::PixelFormat::f16_RGB, true);
    PrefilterMap->SamplingLinear(true);
    EnvCubemap->Activate(0);
    grapho::gl3::GeneratePrefilterMap(cubeRenderer, PrefilterMap->texture_);
    PrefilterMap->GenerateMipmap();

    // brdefLUT
    BrdfLUTTexture = grapho::gl3::GenerateBrdfLUTTexture();

    // ubo
    LightsUbo = grapho::gl3::Ubo::Create(sizeof(Lights), nullptr);

    // skybox
    auto cube = grapho::mesh::Cube();
    auto vbo =
      grapho::gl3::Vbo::Create(cube->Vertices.Size(), cube->Vertices.Data());
    std::shared_ptr<grapho::gl3::Vbo> slots[]{ vbo };
    Cube = grapho::gl3::Vao::Create(cube->Layouts, slots);
    CubeDrawCount = cube->Vertices.Count;

#include "shaders/background_fs.h"
#include "shaders/background_vs.h"
    auto backgroundShader =
      grapho::gl3::ShaderProgram::Create(BACKGROUND_VS, BACKGROUND_FS);
    if (!backgroundShader) {
      throw std::runtime_error(backgroundShader.error());
    }
    BackgroundShader = *backgroundShader;
    BackgroundShader->Use();
    BackgroundShader->Uniform("environmentMap")->SetInt(0);
  }

  void Activate()
  {
    IrradianceMap->Activate(0);
    PrefilterMap->Activate(1);
    BrdfLUTTexture->Activate(2);
    LightsUbo->Upload(lights);
    LightsUbo->SetBindingPoint(UBO_LIGHTS_BINDING);
  }

  void DrawSkybox(const DirectX::XMFLOAT4X4& projection,
                  const DirectX::XMFLOAT4X4& view)
  {
    auto isCull = glIsEnabled(GL_CULL_FACE);
    if(isCull)
    {
      glDisable(GL_CULL_FACE);
    }
    
    EnvCubemap->Activate(0);
    // skybox.Draw(projection, view);

    // render skybox (render as last to prevent overdraw)
    BackgroundShader->Use();
    BackgroundShader->Uniform("projection")->SetMat4(projection);
    BackgroundShader->Uniform("view")->SetMat4(view);
    // glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap); // display irradiance
    // map glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap); // display
    // prefilter map
    Cube->Draw(GL_TRIANGLES, CubeDrawCount);

    // render BRDF map to screen
    // brdfShader.Use();
    // renderQuad();
    if(isCull)
    {
      glEnable(GL_CULL_FACE);
    }
  }
};

inline DirectX::XMFLOAT3X3
TransposeInv(const DirectX::XMFLOAT4X4& _m)
{
  auto m = DirectX::XMLoadFloat4x4(&_m);
  DirectX::XMVECTOR det;
  auto ti = DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(&det, m));
  DirectX::XMFLOAT3X3 mat3;
  DirectX::XMStoreFloat3x3(&mat3, ti);
  return mat3;
}

struct PbrMaterial
{
  std::shared_ptr<grapho::gl3::ShaderProgram> Shader;
  std::shared_ptr<grapho::gl3::Texture> AlbedoMap;
  std::shared_ptr<grapho::gl3::Texture> NormalMap;
  std::shared_ptr<grapho::gl3::Texture> MetallicMap;
  std::shared_ptr<grapho::gl3::Texture> RoughnessMap;
  std::shared_ptr<grapho::gl3::Texture> AOMap;

  PbrMaterial()
  {
#include "shaders/pbr_fs.h"
#include "shaders/pbr_vs.h"
    auto shader = grapho::gl3::ShaderProgram::Create(PBR_VS, PBR_FS);
    if (!shader) {
      throw std::runtime_error(shader.error());
    }
    Shader = *shader;
    Shader->Use();
    Shader->Uniform("irradianceMap")->SetInt(0);
    Shader->Uniform("prefilterMap")->SetInt(1);
    Shader->Uniform("brdfLUT")->SetInt(2);
    Shader->Uniform("albedoMap")->SetInt(3);
    Shader->Uniform("normalMap")->SetInt(4);
    Shader->Uniform("metallicMap")->SetInt(5);
    Shader->Uniform("roughnessMap")->SetInt(6);
    Shader->Uniform("aoMap")->SetInt(7);
  }

  ~PbrMaterial() {}
  PbrMaterial(const PbrMaterial&) = delete;
  PbrMaterial& operator=(const PbrMaterial&) = delete;

  static std::shared_ptr<PbrMaterial> Create(
    const std::shared_ptr<grapho::gl3::Texture>& albedo,
    const std::shared_ptr<grapho::gl3::Texture>& normal,
    const std::shared_ptr<grapho::gl3::Texture>& metallic,
    const std::shared_ptr<grapho::gl3::Texture>& roughness,
    const std::shared_ptr<grapho::gl3::Texture>& ao)
  {
    auto ptr = std::make_shared<PbrMaterial>();
    ptr->AlbedoMap = albedo;
    ptr->NormalMap = normal;
    ptr->MetallicMap = metallic;
    ptr->RoughnessMap = roughness;
    ptr->AOMap = ao;
    return ptr;
  }

  void Activate(const DirectX::XMFLOAT4X4& projection,
                const DirectX::XMFLOAT4X4& view,
                const DirectX::XMFLOAT4X4& model,
                const DirectX::XMFLOAT3& cameraPos,
                uint32_t UBO_LIGHTS_BINDING)
  {
    if (AlbedoMap) {
      AlbedoMap->Activate(3);
    }
    if (NormalMap) {
      NormalMap->Activate(4);
    }
    if (MetallicMap) {
      MetallicMap->Activate(5);
    }
    if (RoughnessMap) {
      RoughnessMap->Activate(6);
    }
    if (AOMap) {
      AOMap->Activate(7);
    }

    Shader->Use();
    // auto view = cameraViewMatrix();
    Shader->Uniform("view")->SetMat4(view);
    // auto cameraPos = cameraPosition();
    Shader->Uniform("camPos")->SetFloat3(cameraPos);
    // initialize static shader uniforms before rendering
    Shader->Uniform("projection")->SetMat4(projection);

    auto uboBlock = Shader->UboBlockIndex("lights");
    Shader->UboBind(*uboBlock, UBO_LIGHTS_BINDING);

    // DirectX::XMFLOAT4X4 model;
    // DirectX::XMStoreFloat4x4(
    //   &model, DirectX::XMMatrixTranslation(position.x, position.y,
    //   position.z));
    Shader->Uniform("model")->SetMat4(model);
    Shader->Uniform("normalMatrix")->SetMat3(TransposeInv(model));
  }
};
}
}
