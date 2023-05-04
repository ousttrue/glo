#include <GL/glew.h>

#include "drawable.h"
#include "pbrmaterial.h"
#include <grapho/gl3/vao.h>
#include <grapho/vertexlayout.h>

Drawable::Drawable()
{
  auto sphere = grapho::Mesh::Sphere();

  auto vbo = grapho::gl3::Vbo::Create(sphere->Vertices);
  std::shared_ptr<grapho::gl3::Vbo> slots[]{
    vbo,
  };
  auto ibo = grapho::gl3::Ibo::Create(sphere->Indices);

  Mesh = grapho::gl3::Vao::Create(sphere->Layouts, slots, ibo);
  MeshDrawCount = sphere->Indices.size();
}

void
Drawable::Draw(const DirectX::XMFLOAT4X4& projection,
               const DirectX::XMFLOAT4X4& view,
               const DirectX::XMFLOAT3& cameraPos,
               const Lights& lights)
{
  Material->Activate(projection, view, Position, cameraPos, lights);
  Mesh->Draw(GL_TRIANGLE_STRIP, MeshDrawCount);
}
