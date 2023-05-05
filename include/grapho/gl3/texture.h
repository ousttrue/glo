#pragma once
#include "../pixelformat.h"
#include <assert.h>
#include <expected>
#include <memory>
#include <stdint.h>
#include <string>

namespace grapho::gl3 {

inline std::expected<uint32_t, std::string>
GLImageFormat(PixelFormat format)
{
  switch (format) {
    case PixelFormat::f16_RGB:
      return GL_RGB16F;
    case PixelFormat::u8_RGBA:
      return GL_RGBA;
    case PixelFormat::u8_RGB:
      return GL_RGB;
    case PixelFormat::u8_R:
      return GL_RED;
    defualt:
      break;
  }

  return std::unexpected{ "unknown PixelFormat" };
}

inline uint32_t
GLInternalFormat(PixelFormat format)
{
  switch (format) {
    case PixelFormat::u8_RGBA:
      return GL_RGBA;

    case PixelFormat::u8_R:
      return GL_RED;

    defualt:
      break;
  }
  return GL_RGB;
}

struct Texture
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
                                         PixelFormat format,
                                         const uint8_t* pixels = nullptr,
                                         bool useFloat = false)
  {
    uint32_t texture;
    glGenTextures(1, &texture);
    auto ptr = std::shared_ptr<Texture>(new Texture(texture));
    ptr->Upload(width, height, format, pixels, useFloat);
    return ptr;
  }

  void Upload(int width,
              int height,
              PixelFormat format,
              const uint8_t* pixels,
              bool useFloat)
  {
    width_ = width;
    height_ = height;
    SamplingLinear();
    WrapClamp();
    Bind();
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 *GLImageFormat(format),
                 width,
                 height,
                 0,
                 GLInternalFormat(format),
                 useFloat ? GL_FLOAT : GL_UNSIGNED_BYTE,
                 pixels);
    glGenerateMipmap(GL_TEXTURE_2D);
    UnBind();
  }

  void WrapClamp()
  {
    Bind();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    UnBind();
  }

  void WrapRepeat()
  {
    Bind();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    UnBind();
  }

  void SamplingPoint()
  {
    Bind();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    UnBind();
  }

  void SamplingLinear(bool mip = false)
  {
    Bind();
    if (mip) {
      glTexParameteri(
        GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    } else {
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    UnBind();
  }

  void Activate(uint32_t unit)
  {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, texture_);
  }

  void Bind() { glBindTexture(GL_TEXTURE_2D, texture_); }

  void UnBind() { glBindTexture(GL_TEXTURE_2D, 0); }
};
}
