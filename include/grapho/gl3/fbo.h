#pragma once
#include "../viewport.h"
#include "texture.h"
#include <assert.h>

namespace grapho::gl3 {

void
ClearViewport(const Viewport& vp, bool applyAlpha = false)
{
  glViewport(0, 0, vp.Width, vp.Height);
  glScissor(0, 0, vp.Width, vp.Height);
  if (applyAlpha) {
    glClearColor(vp.Color[0] * vp.Color[3],
                 vp.Color[1] * vp.Color[3],
                 vp.Color[2] * vp.Color[3],
                 vp.Color[3]);
  } else {
    glClearColor(vp.Color[0], vp.Color[1], vp.Color[2], vp.Color[3]);
  }
  glClearDepth(vp.Depth);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

struct Fbo
{
  uint32_t m_fbo = 0;
  uint32_t m_rbo = 0;
  Fbo()
  {
    // this->texture = Texture::Create(width, height);
    glGenFramebuffers(1, &m_fbo);
    // glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
  }
  ~Fbo()
  {
    glDeleteFramebuffers(1, &m_fbo);
    if (m_rbo) {
      glDeleteRenderbuffers(1, &m_rbo);
    }
  }
  Fbo(const Fbo&) = delete;
  Fbo& operator=(const Fbo&) = delete;

  void Bind() { glBindFramebuffer(GL_FRAMEBUFFER, m_fbo); }
  void Unbind() { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

  void AttachDepth(int width, int height)
  {
    assert(m_rbo == 0);
    glGenRenderbuffers(1, &m_rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, m_rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
    glFramebufferRenderbuffer(
      GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_rbo);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }

  void AttachTexture2D(uint32_t texture, int mipLevel = 0)
  {
    Bind();
    glFramebufferTexture2D(
      GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, mipLevel);
    // uint32_t buffers[] = { GL_COLOR_ATTACHMENT0 };
    // glDrawBuffers(1, buffers);
  }

  void AttachCubeMap(int i, uint32_t texture, int mipLevel = 0)
  {
    Bind();
    glFramebufferTexture2D(GL_FRAMEBUFFER,
                           GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                           texture,
                           mipLevel);
  }
};

}
