#pragma once
#include "texture.h"

namespace grapho::gl3 {

struct Fbo
{
  int width_ = 0;
  int height_ = 0;
  uint32_t fbo_ = 0;
  uint32_t rbo_ = 0;

private:
  Fbo(int width, int height, bool use_depth)
    : width_(width)
    , height_(height)
  {
    // this->texture = Texture::Create(width, height);
    glGenFramebuffers(1, &this->fbo_);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_);

    if (use_depth) {
      glGenRenderbuffers(1, &rbo_);
      glBindRenderbuffer(GL_RENDERBUFFER, rbo_);
      glRenderbufferStorage(
        GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
      glFramebufferRenderbuffer(
        GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo_);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }

public:
  Fbo(const Fbo&) = delete;
  Fbo& operator=(const Fbo&) = delete;
  ~Fbo() { glDeleteFramebuffers(1, &fbo_); }
  static std::shared_ptr<Fbo> Create(int width,
                                     int height,
                                     bool use_depth = true)
  {
    return std::shared_ptr<Fbo>(new Fbo(width, height, use_depth));
  }
  void Bind() { glBindFramebuffer(GL_FRAMEBUFFER, fbo_); }
  void Unbind() { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

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

  void Clear(const float color[4] = {}, float depth = 1.0f)
  {
    Bind();
    glViewport(0, 0, width_, height_);
    glScissor(0, 0, width_, height_);
    if (color) {
      glClearColor(color[0] * color[3],
                   color[1] * color[3],
                   color[2] * color[3],
                   color[3]);
    }
    if (rbo_) {
      glClearDepth(depth);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      // glDepthFunc(GL_LESS);
    } else {
      glClear(GL_COLOR_BUFFER_BIT);
    }
  }
};

}
