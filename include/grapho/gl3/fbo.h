#pragma once
#include "../camera/viewport.h"
#include "error_check.h"
#include "texture.h"
#include <assert.h>

namespace grapho::gl3 {

struct ClearParam
{
  bool Depth = true;
  bool ApplyAlpha = false;
};

inline void
ClearViewport(const camera::Viewport& vp, const ClearParam& param = {})
{
  glViewport(0, 0, static_cast<int>(vp.Width), static_cast<int>(vp.Height));
  assert(!TryGetError());
  glScissor(0, 0, static_cast<int>(vp.Width), static_cast<int>(vp.Height));
  assert(!TryGetError());
  if (param.ApplyAlpha) {
    glClearColor(vp.Color[0] * vp.Color[3],
                 vp.Color[1] * vp.Color[3],
                 vp.Color[2] * vp.Color[3],
                 vp.Color[3]);
  } else {
    glClearColor(vp.Color[0], vp.Color[1], vp.Color[2], vp.Color[3]);
  }
  assert(!TryGetError());
  if (param.Depth) {
    glClearDepth(vp.Depth);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  } else {
    glClear(GL_COLOR_BUFFER_BIT);
  }
  assert(!TryGetError());
}

struct Fbo
{
  uint32_t m_fbo = 0;
  uint32_t m_rbo = 0;
  Fbo() { glGenFramebuffers(1, &m_fbo); }
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
    Bind();
    if (!m_rbo) {
      glGenRenderbuffers(1, &m_rbo);
    }
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

struct FboHolder
{
  std::shared_ptr<grapho::gl3::Texture> FboTexture;
  grapho::gl3::Fbo Fbo;

  template<typename T>
    requires(sizeof(T) == sizeof(float) * 4)
  uint32_t Bind(int width, int height, const T& color)
  {
    if (!FboTexture || FboTexture->Width() != width ||
        FboTexture->Height() != height) {
      FboTexture = grapho::gl3::Texture::Create({
        width,
        height,
        grapho::PixelFormat::u8_RGBA,

      });
      Fbo.AttachDepth(width, height);
      Fbo.AttachTexture2D(FboTexture->Handle());
    }

    Fbo.Bind();
    grapho::gl3::ClearViewport({
      .Width = width,
      .Height = height,
      .Color = *((const std::array<float, 4>*)&color),
    });

    return FboTexture->Handle();
  }

  void Unbind() { Fbo.Unbind(); }
};

struct RenderTarget
{
  std::shared_ptr<Fbo> Fbo;
  std::shared_ptr<Texture> FboTexture;

  uint32_t Begin(float width, float height, const float color[4])
  {
    if (width == 0 || height == 0) {
      return 0;
    }
    if (!Fbo) {
      Fbo = std::make_shared<grapho::gl3::Fbo>();
    }

    if (FboTexture) {
      if (FboTexture->Width() != width || FboTexture->Height() != height) {
        FboTexture = nullptr;
      }
    }
    if (!FboTexture) {
      FboTexture = grapho::gl3::Texture::Create({
        static_cast<int>(width),
        static_cast<int>(height),
        grapho::PixelFormat::u8_RGB,
        grapho::ColorSpace::Linear,
      });
      Fbo->AttachTexture2D(FboTexture->Handle());
      Fbo->AttachDepth(static_cast<int>(width), static_cast<int>(height));
    }

    Fbo->Bind();
    grapho::gl3::ClearViewport({
      .Width = width,
      .Height = height,
      .Color = { color[0], color[1], color[2], color[3] },
      .Depth = 1.0f,
    });

    return FboTexture->Handle();
  }

  void End() { Fbo->Unbind(); }
};

}
