#pragma once
#include <memory>
namespace glo {
class Texture
{
  uint32_t texture_;
  int width_ = 0;
  int height_ = 0;
  Texture(uint32_t texture)
    : texture_(texture)
  {
  }

public:
  static std::shared_ptr<Texture> Create(int width,
                                         int height,
                                         const uint8_t* pixels)
  {
    uint32_t texture;
    glGenTextures(1, &texture);
    auto ptr = std::shared_ptr<Texture>(new Texture(texture));
    ptr->Upload(width, height, pixels);
    return ptr;
  }

  void Upload(int width, int height, const uint8_t* pixels)
  {
    width_ = width;
    height_ = height;
    Bind();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGBA,
                 width,
                 height,
                 0,
                 GL_RGBA,
                 GL_UNSIGNED_BYTE,
                 pixels);
    glGenerateMipmap(GL_TEXTURE_2D);
    UnBind();
  }

  void Bind(int unit = 0)
  {
    glBindTexture(GL_TEXTURE_2D, texture_);
    // glActiveTexture(GL_TEXTURE0 + unit);
  }
  void UnBind() { glBindTexture(GL_TEXTURE_2D, 0); }
};
}
