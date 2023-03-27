#pragma once
#include "texture.h"

namespace glo {

struct Fbo
{
  uint32_t fbo_ = 0;
  uint32_t depth_ = 0;
  std::shared_ptr<Texture> texture;

private:
  Fbo(int width, int height, bool use_depth)
  {
    this->texture = Texture::Create(width, height);
    glGenFramebuffers(1, &this->fbo_);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
    glFramebufferTexture2D(GL_FRAMEBUFFER,
                           GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D,
                           texture->texture_,
                           0);
    uint32_t buffers[] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, buffers);

    if (use_depth) {
      glGenRenderbuffers(1, &depth_);
      glBindRenderbuffer(GL_RENDERBUFFER, depth_);
      glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
      glFramebufferRenderbuffer(
        GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_);
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

  void Clear(const float color[4], float depth = 1.0f)
  {
    Bind();
    glViewport(0, 0, texture->width_, texture->height_);
    glScissor(0, 0, texture->width_, texture->height_);
    glClearColor(
      color[0] * color[3], color[1] * color[3], color[2] * color[3], color[3]);
    if (depth_) {
      glClearDepth(depth);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    } else {
      glClear(GL_COLOR_BUFFER_BIT);
    }
    glDepthFunc(GL_LESS);
  }
};

}
