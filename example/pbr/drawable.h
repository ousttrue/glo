#pragma once
#include <DirectXMath.h>
#include <filesystem>
#include <grapho/gl3/material.h>
#include <grapho/gl3/ubo.h>
#include <memory>
#include <vector>

namespace grapho {
namespace gl3 {
struct Vao;
struct Material;
}
}

struct Drawable
{
  std::shared_ptr<grapho::gl3::Vao> Mesh;
  uint32_t MeshDrawCount = 0;
  uint32_t MeshDrawMode = 0;
  std::shared_ptr<grapho::gl3::Material> Material;
  DirectX::XMFLOAT3 Position = {};
  grapho::gl3::Material::LocalVars Vars;
  std::shared_ptr<grapho::gl3::Ubo> Ubo;

  Drawable();
  ~Drawable() {}
  void Draw(uint32_t world_ubo_binding);
};

struct Scene
{
  std::vector<std::shared_ptr<Drawable>> Drawables;
  void Load(const std::filesystem::path& baseDir);
};
