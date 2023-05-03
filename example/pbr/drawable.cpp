#include <Gl/glew.h>

#include "drawable.h"
#include "mesh.h"
#include "pbrmaterial.h"
#include <grapho/gl3/shader.h>
#include <grapho/gl3/vao.h>

static DirectX::XMFLOAT3X3
TransposeInv(const DirectX::XMFLOAT4X4& _m)
{
  auto m = DirectX::XMLoadFloat4x4(&_m);
  DirectX::XMVECTOR det;
  auto ti = DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(&det, m));
  DirectX::XMFLOAT3X3 mat3;
  DirectX::XMStoreFloat3x3(&mat3, ti);
  return mat3;
}

Drawable::Drawable()
{
  {
    auto shader = grapho::gl3::ShaderProgram::CreateFromPath("2.2.2.pbr.vs",
                                                             "2.2.2.pbr.fs");
    if (!shader) {
      throw std::runtime_error(shader.error());
    }
    Shader = *shader;
    Shader->Use();
    Shader->Uniform("irradianceMap")->SetInt(0);
    Shader->Uniform("prefilterMap")->SetInt(1);
    Shader->Uniform("brdfLUT")->SetInt(2);
    Shader->Uniform("albedoMap")->SetInt(3);
    Shader->Uniform("normalMap")->SetInt(4);
    Shader->Uniform("metallicMap")->SetInt(5);
    Shader->Uniform("roughnessMap")->SetInt(6);
    Shader->Uniform("aoMap")->SetInt(7);
  }
  {
    auto sphere = Mesh::Sphere();

    auto vbo = grapho::gl3::Vbo::Create(sphere->Vertices);
    std::shared_ptr<grapho::gl3::Vbo> slots[]{
      vbo,
    };
    auto ibo = grapho::gl3::Ibo::Create(sphere->Indices);

    Mesh = grapho::gl3::Vao::Create(sphere->Layouts, slots, ibo);
    MeshDrawCount = sphere->Indices.size();
  }
}

void
Drawable::Draw(const DirectX::XMFLOAT4X4& projection,
               const DirectX::XMFLOAT4X4& view,
               const DirectX::XMFLOAT3& cameraPos,
               std::span<const std::shared_ptr<Light>> lights)
{
  for (unsigned int i = 0; i < lights.size(); ++i) {
    // glm::vec3 newPos =
    //   lightPositions[i] + glm::vec3(sin(currentFrame * 5.0) * 5.0, 0.0,
    //   0.0);
    auto newPos = lights[i]->Position;
    Shader->Uniform("lightPositions[" + std::to_string(i) + "]")
      ->SetFloat3(newPos);
    Shader->Uniform("lightColors[" + std::to_string(i) + "]")
      ->SetFloat3(lights[i]->Color);

    // model = glm::mat4(1.0f);
    // model = glm::translate(model, newPos);
    // model = glm::scale(model, glm::vec3(0.5f));
    // PbrShader->Uniform("model")->SetMat4(model);
    // PbrShader->Uniform("normalMatrix")
    //   ->SetMat3(glm::transpose(glm::inverse(glm::mat3(model))));
    // Sphere->Draw(GL_TRIANGLE_STRIP, SphereDrawCount);
  }

  Shader->Use();
  // auto view = cameraViewMatrix();
  Shader->Uniform("view")->SetMat4(view);
  // auto cameraPos = cameraPosition();
  Shader->Uniform("camPos")->SetFloat3(cameraPos);
  // initialize static shader uniforms before rendering
  Shader->Uniform("projection")->SetMat4(projection);

  DirectX::XMFLOAT4X4 model;
  DirectX::XMStoreFloat4x4(
    &model, DirectX::XMMatrixTranslation(Position.x, Position.y, Position.z));
  Shader->Uniform("model")->SetMat4(model);
  Shader->Uniform("normalMatrix")->SetMat3(TransposeInv(model));
  Material->Bind();
  Mesh->Draw(GL_TRIANGLE_STRIP, MeshDrawCount);
}
