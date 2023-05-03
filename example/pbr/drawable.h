#pragma once
#include <DirectXMath.h>
#include <memory>
#include <span>

struct Light
{
  DirectX::XMFLOAT3 Position;
  DirectX::XMFLOAT3 Color;
};

namespace grapho {
namespace gl3 {
class ShaderProgram;
struct Vao;
}
}

struct PbrMaterial;
struct Drawable
{
  std::shared_ptr<grapho::gl3::Vao> Mesh;
  uint32_t MeshDrawCount = 0;
  std::shared_ptr<grapho::gl3::ShaderProgram> Shader;
  std::shared_ptr<PbrMaterial> Material;
  DirectX::XMFLOAT3 Position = {};

  Drawable();
  ~Drawable() {}
  void Draw(const DirectX::XMFLOAT4X4& projection,
            const DirectX::XMFLOAT4X4& view,
            const DirectX::XMFLOAT3& cameraPos,
            std::span<const std::shared_ptr<Light>> lights);
};
