#pragma once
#include <DirectXMath.h>
#include <memory>
#include <span>

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

namespace grapho {
namespace gl3 {
class ShaderProgram;
struct Vao;
struct Ubo;
}
}

struct PbrMaterial;
struct Drawable
{
  std::shared_ptr<grapho::gl3::Vao> Mesh;
  uint32_t MeshDrawCount = 0;
  std::shared_ptr<grapho::gl3::ShaderProgram> Shader;
  std::shared_ptr<PbrMaterial> Material;
  std::shared_ptr<grapho::gl3::Ubo> LightsUbo;
  DirectX::XMFLOAT3 Position = {};

  Drawable();
  ~Drawable() {}
  void Draw(const DirectX::XMFLOAT4X4& projection,
            const DirectX::XMFLOAT4X4& view,
            const DirectX::XMFLOAT3& cameraPos,
            const Lights& lights);
};
