#pragma once
#include <DirectXMath.h>
#include <GL/glew.h>
#include <grapho/gl3/shader.h>
#include <grapho/gl3/vao.h>
#include <learnopengl/shader.h>
#include <memory>

struct PbrMaterial;
class Renderer
{
  // timing
  float deltaTime = 0.0f;
  float lastFrame = 0.0f;

  std::shared_ptr<grapho::gl3::ShaderProgram> PbrShader;
  std::shared_ptr<grapho::gl3::ShaderProgram> BackgroundShader;
  std::shared_ptr<grapho::gl3::Vao> Sphere;
  uint32_t SphereDrawCount = 0;
  std::shared_ptr<grapho::gl3::Vao> Cube;
  uint32_t CubeDrawCount = 0;

  unsigned int envCubemap;
  unsigned int irradianceMap;
  unsigned int prefilterMap;
  unsigned int brdfLUTTexture;

  std::shared_ptr<PbrMaterial> Iron;
  std::shared_ptr<PbrMaterial> Gold;
  std::shared_ptr<PbrMaterial> Grass;
  std::shared_ptr<PbrMaterial> Plastic;
  std::shared_ptr<PbrMaterial> Wall;

  // lights
  // ------
  glm::vec3 lightPositions[4] = {
    glm::vec3(-10.0f, 10.0f, 10.0f),
    glm::vec3(10.0f, 10.0f, 10.0f),
    glm::vec3(-10.0f, -10.0f, 10.0f),
    glm::vec3(10.0f, -10.0f, 10.0f),
  };

  glm::vec3 lightColors[4] = { glm::vec3(300.0f, 300.0f, 300.0f),
                               glm::vec3(300.0f, 300.0f, 300.0f),
                               glm::vec3(300.0f, 300.0f, 300.0f),
                               glm::vec3(300.0f, 300.0f, 300.0f) };

public:
  Renderer();
  ~Renderer();
  void Render(float crrentFrame,
              int w,
              int h,
              const DirectX::XMFLOAT4X4& projectin,
              const DirectX::XMFLOAT4X4& view,
              const DirectX::XMFLOAT3& cameraPos);
};
