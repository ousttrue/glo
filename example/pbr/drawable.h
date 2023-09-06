#pragma once
#include <filesystem>
#include <grapho/dxmath_stub.h>
#include <grapho/vars.h>
#include <memory>
#include <vector>

namespace grapho {
namespace gl3 {
struct Vao;
struct Ubo;
class ShaderProgram;
class Texture;
}
}

struct Drawable
{
  std::shared_ptr<grapho::gl3::Vao> Mesh;
  uint32_t MeshDrawCount = 0;
  uint32_t MeshDrawMode = 0;
  std::shared_ptr<grapho::gl3::ShaderProgram> Shader;
  DirectX::XMFLOAT3 Position = {};
  grapho::LocalVars Vars;
  std::shared_ptr<grapho::gl3::Ubo> Ubo;
  std::vector<std::shared_ptr<grapho::gl3::Texture>> Textures;

  Drawable();
  ~Drawable() {}
  void Draw(uint32_t world_ubo_binding);

  static std::shared_ptr<Drawable> Load(const std::filesystem::path& baseDir,
                                        const DirectX::XMFLOAT3& position);
};
