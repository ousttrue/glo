#include "mesh.h"
#include <GL/glew.h>
#include <grapho/gl3/shader.h>
#include <grapho/gl3/vao.h>

#include "skybox.h"

Skybox::Skybox(uint32_t cubemap)
  : envCubemap(cubemap)
{
  auto cube = Mesh::Cube();
  auto vbo = grapho::gl3::Vbo::Create(cube->Vertices);
  std::shared_ptr<grapho::gl3::Vbo> slots[]{ vbo };
  Cube = grapho::gl3::Vao::Create(cube->Layouts, slots);
  CubeDrawCount = cube->Vertices.size();

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
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
  // glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap); // display irradiance
  // map glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap); // display
  // prefilter map
  Cube->Draw(GL_TRIANGLES, CubeDrawCount);

  // render BRDF map to screen
  // brdfShader.Use();
  // renderQuad();
}
