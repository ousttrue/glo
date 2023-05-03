#pragma once
#include <GL/glew.h>

#include <DirectXMath.h>
#include <grapho/gl3/shader.h>
#include <grapho/gl3/vao.h>
#include <stdint.h>

struct Environment
{
  unsigned int envCubemap;
  unsigned int irradianceMap;
  unsigned int prefilterMap;
  unsigned int brdfLUTTexture;
  std::shared_ptr<grapho::gl3::ShaderProgram> BackgroundShader;
  std::shared_ptr<grapho::gl3::Vao> Cube;
  uint32_t CubeDrawCount = 0;

  Environment();
  void Bind();
  void DrawSkybox(const DirectX::XMFLOAT4X4& projection,
                  const DirectX::XMFLOAT4X4& view);
};
