#pragma once
#include "error_handler.h"
#include <functional>
#include <memory>
#include <optional>
#include <span>
#include <stdint.h>
#include <string_view>

namespace glo {

template<typename T>
concept Mat4 = sizeof(T) == 4 * 16;

class ShaderProgram
{
  uint32_t program_ = 0;

  ShaderProgram(uint32_t program);

public:
  ~ShaderProgram();
  static std::shared_ptr<ShaderProgram> Create(const ErrorHandler& onError,
                                               std::span<std::string_view> vs,
                                               std::span<std::string_view> fs);
  static std::shared_ptr<ShaderProgram> Create(const ErrorHandler& onError,
                                               std::string_view vs,
                                               std::string_view fs)
  {
    std::string_view vss[] = { vs };
    std::string_view fss[] = { fs };
    return Create(onError, vss, fss);
  }
  void Bind();
  void Unbind();
  std::optional<uint32_t> AttributeLocation(const char* name);
  std::optional<uint32_t> UniformLocation(const char* name);

  // location
  void _SetUniformMatrix(const ErrorHandler& onError,
                         uint32_t location,
                         const float m[16]);
  template<Mat4 T>
  void SetUniformMatrix(const ErrorHandler& onError,
                        uint32_t location,
                        const T& t)
  {
    _SetUniformMatrix(onError, location, (float*)&t);
  }

  // name
  void _SetUniformMatrix(const ErrorHandler& onError,
                         const char* name,
                         const float m[16]);
  template<Mat4 T>
  void SetUniformMatrix(const ErrorHandler& onError,
                        const char* name,
                        const T& t)
  {
    _SetUniformMatrix(onError, name, (float*)&t);
  }
};
}
