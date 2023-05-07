#pragma once
#include "../image.h"
#include <assert.h>
#include <expected>
#include <memory>
#include <stdint.h>
#include <string>

namespace grapho {
namespace gl3 {

inline std::expected<uint32_t, std::string>
GLImageFormat(PixelFormat format, ColorSpace colorspace)
{
  if (colorspace == ColorSpace::Linear) {
    switch (format) {
      case PixelFormat::f32_RGB:
        return GL_RGB32F;
      case PixelFormat::f16_RGB:
        return GL_RGB16F;
      case PixelFormat::u8_RGBA:
        return GL_RGBA;
      case PixelFormat::u8_RGB:
        return GL_RGB;
      case PixelFormat::u8_R:
        return GL_RED;
      default:
        break;
    }
  } else {
    switch (format) {
      case PixelFormat::u8_RGBA:
        return GL_SRGB8_ALPHA8;
      case PixelFormat::u8_RGB:
        return GL_SRGB8;
      default:
        assert(false);
        break;
    }
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
  static std::shared_ptr<Texture> Create(const Image& data,
                                         bool useFloat = false)
  {
    uint32_t texture;
    glGenTextures(1, &texture);
    auto ptr = std::shared_ptr<Texture>(new Texture(texture));
    ptr->Upload(data, useFloat);
    return ptr;
  }

  void Upload(const Image& data, bool useFloat)
  {
    width_ = data.Width;
    height_ = data.Height;
    SamplingLinear();
    WrapClamp();
    Bind();
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 *GLImageFormat(data.Format, data.ColorSpace),
                 width_,
                 height_,
                 0,
                 GLInternalFormat(data.Format),
                 useFloat ? GL_FLOAT : GL_UNSIGNED_BYTE,
                 data.Pixels);
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

struct Cubemap
{
  uint32_t texture_;
  int width_ = 0;
  int height_ = 0;
  Cubemap(uint32_t texture)
    : texture_(texture)
  {
  }

  static std::shared_ptr<Cubemap> Create(const Image& data,
                                         bool useFloat = false)
  {
    uint32_t texture;
    glGenTextures(1, &texture);
    auto ptr = std::shared_ptr<Cubemap>(new Cubemap(texture));
    ptr->width_ = data.Width;
    ptr->height_ = data.Height;
    ptr->Bind();
    for (unsigned int i = 0; i < 6; ++i) {
      glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                   0,
                   *GLImageFormat(data.Format, data.ColorSpace),
                   ptr->width_,
                   ptr->height_,
                   0,
                   GLInternalFormat(data.Format),
                   useFloat ? GL_FLOAT : GL_UNSIGNED_BYTE,
                   nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    ptr->UnBind();
    ptr->SamplingLinear();

    return ptr;
  }

  void GenerateMipmap()
  {
    // then let OpenGL generate mipmaps from first mip face (combatting visible
    // dots artifact)
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture_);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
  }

  void Bind() { glBindTexture(GL_TEXTURE_CUBE_MAP, texture_); }
  void UnBind() { glBindTexture(GL_TEXTURE_CUBE_MAP, 0); }

  void SamplingLinear(bool mip = false)
  {
    Bind();
    if (mip) {
      // enable pre-filter mipmap sampling (combatting visible dots artifact)
      glTexParameteri(
        GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    } else {
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    UnBind();
  }

  void Activate(uint32_t unit)
  {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture_);
  }
};

}
}
