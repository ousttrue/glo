#pragma once
#include <DirectXMath.h>
#include <filesystem>
#include <memory>
#include <vector>

namespace grapho {
namespace gl3 {
struct Vao;
struct PbrMaterial;
}
}

struct Drawable
{
  std::shared_ptr<grapho::gl3::Vao> Mesh;
  uint32_t MeshDrawCount = 0;
  uint32_t MeshDrawMode = 0;
  std::shared_ptr<grapho::gl3::PbrMaterial> Material;
  DirectX::XMFLOAT3 Position = {};

  Drawable();
  ~Drawable() {}
  void Draw(const DirectX::XMFLOAT4X4& projection,
            const DirectX::XMFLOAT4X4& view,
            const DirectX::XMFLOAT3& cameraPos,
            uint32_t UBO_LIGHTS_BINDING);
};

struct Scene
{
  std::vector<std::shared_ptr<Drawable>> Drawables;
  void Load(const std::filesystem::path& baseDir);
};
