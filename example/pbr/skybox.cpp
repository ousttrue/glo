#include <GL/glew.h>
#include <grapho/gl3/shader.h>
#include <grapho/gl3/vao.h>
#include <grapho/mesh.h>

#include "skybox.h"

Skybox::Skybox()
{
  auto cube = grapho::mesh::Cube();
  auto vbo = grapho::gl3::Vbo::Create(cube->Vertices.Size(), cube->Vertices.Data());
  std::shared_ptr<grapho::gl3::Vbo> slots[]{ vbo };
  Cube = grapho::gl3::Vao::Create(cube->Layouts, slots);
  CubeDrawCount = cube->Vertices.Count;

  auto backgroundShader = grapho::gl3::ShaderProgram::CreateFromPath(
    "2.2.2.background.vs", "2.2.2.background.fs");
  if (!backgroundShader) {
    throw std::runtime_error(backgroundShader.error());
  }
  BackgroundShader = *backgroundShader;
  BackgroundShader->Use();
  BackgroundShader->Uniform("environmentMap")->SetInt(0);
}

void
Skybox::Draw(const DirectX::XMFLOAT4X4& projection,
             const DirectX::XMFLOAT4X4& view)
{
  // render skybox (render as last to prevent overdraw)
  BackgroundShader->Use();
  BackgroundShader->Uniform("projection")->SetMat4(projection);
  BackgroundShader->Uniform("view")->SetMat4(view);
  // glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap); // display irradiance
  // map glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap); // display
  // prefilter map
  Cube->Draw(GL_TRIANGLES, CubeDrawCount);

  // render BRDF map to screen
  // brdfShader.Use();
  // renderQuad();
}
