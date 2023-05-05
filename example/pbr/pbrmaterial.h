#pragma once
#include <DirectXMath.h>
#include <filesystem>
#include <memory>
#include <string_view>

namespace grapho {
namespace gl3 {
class ShaderProgram;
struct Ubo;
struct Texture;
}
}

struct PbrMaterial
{
  std::shared_ptr<grapho::gl3::ShaderProgram> Shader;
  std::shared_ptr<grapho::gl3::Texture> AlbedoMap;
  std::shared_ptr<grapho::gl3::Texture> NormalMap;
  std::shared_ptr<grapho::gl3::Texture> MetallicMap;
  std::shared_ptr<grapho::gl3::Texture> RoughnessMap;
  std::shared_ptr<grapho::gl3::Texture> AOMap;

  PbrMaterial();
  ~PbrMaterial() {}
  PbrMaterial(const PbrMaterial&) = delete;
  PbrMaterial& operator=(const PbrMaterial&) = delete;

  static std::shared_ptr<PbrMaterial> Create(
    const std::filesystem::path& texture_path,
    const std::filesystem::path& normal,
    const std::filesystem::path& metallic,
    const std::filesystem::path& roughness,
    const std::filesystem::path& ao);
  void Activate(const DirectX::XMFLOAT4X4& projection,
                const DirectX::XMFLOAT4X4& view,
                const DirectX::XMFLOAT3& position,
                const DirectX::XMFLOAT3& cameraPos,
                uint32_t UBO_LIGHTS_BINDING);
};
