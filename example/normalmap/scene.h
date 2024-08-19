#pragma once
#include <glm/glm.hpp>
#include <grapho/vertexlayout.h>
#include <memory>

namespace grapho {
namespace gl3 {
class ShaderProgram;
}
}

class Scene
{
  std::shared_ptr<grapho::gl3::ShaderProgram> Shader;
  unsigned int DiffuseMap = 0;
  unsigned int NormalMap = 0;

  float Time = 0.0f;

  // lighting info
  glm::vec3 LightPos{ 0.5f, 1.0f, 0.3f };

public:
  bool Initialize(const std::string& dir);

  void Render(float deltaTime,
              const grapho::XMFLOAT4X4& projection,
              const grapho::XMFLOAT4X4& view,
              const grapho::XMFLOAT3& cameraPosition);
};
