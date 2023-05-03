#pragma once
#include <DirectXMath.h>
#include <memory>
#include <string_view>

namespace grapho {
namespace gl3 {
class ShaderProgram;
struct Ubo;
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

struct PbrMaterial
{
  std::shared_ptr<grapho::gl3::ShaderProgram> Shader;
  std::shared_ptr<grapho::gl3::Ubo> LightsUbo;
  unsigned int AlbedoMap;
  unsigned int NormalMap;
  unsigned int MetallicMap;
  unsigned int RoughnessMap;
  unsigned int AOMap;

  PbrMaterial();
  ~PbrMaterial() {}
  PbrMaterial(const PbrMaterial&) = delete;
  PbrMaterial& operator=(const PbrMaterial&) = delete;

  static std::shared_ptr<PbrMaterial> Create(std::string_view texture_path,
                                             std::string_view normal,
                                             std::string_view metallic,
                                             std::string_view roughness,
                                             std::string_view ao);
  void Activate(const DirectX::XMFLOAT4X4& projection,
                const DirectX::XMFLOAT4X4& view,
                const DirectX::XMFLOAT3& position,
                const DirectX::XMFLOAT3& cameraPos,
                const Lights& lights);
};
