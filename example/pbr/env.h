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

  Environment();
  void Bind();
};
