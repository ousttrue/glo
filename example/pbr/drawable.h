#pragma once
#include <DirectXMath.h>
#include <memory>

namespace grapho {
namespace gl3 {
struct Vao;
}
}

struct Lights;
struct PbrMaterial;
struct Drawable
{
  std::shared_ptr<grapho::gl3::Vao> Mesh;
  uint32_t MeshDrawCount = 0;
  uint32_t MeshDrawMode = 0;
  std::shared_ptr<PbrMaterial> Material;
  DirectX::XMFLOAT3 Position = {};

  Drawable();
  ~Drawable() {}
  void Draw(const DirectX::XMFLOAT4X4& projection,
            const DirectX::XMFLOAT4X4& view,
            const DirectX::XMFLOAT3& cameraPos,
            const Lights& lights);
};
