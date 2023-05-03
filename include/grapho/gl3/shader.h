#pragma once
#include "../fileutil.h"
#include <expected>
#include <functional>
#include <memory>
#include <optional>
#include <span>
#include <stdint.h>
#include <string>
#include <string_view>

namespace grapho::gl3 {

inline std::expected<GLuint, std::string>
compile(GLenum shaderType, std::span<std::u8string_view> srcs)
{
  auto shader = glCreateShader(shaderType);

  std::vector<const GLchar*> string;
  std::vector<GLint> length;
  for (auto src : srcs) {
    string.push_back((const GLchar*)src.data());
    length.push_back(src.size());
  }
  glShaderSource(shader, srcs.size(), string.data(), length.data());
  glCompileShader(shader);
  GLint isCompiled = 0;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
  if (isCompiled == GL_FALSE) {
    GLint maxLength = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

    // The maxLength includes the NULL character
    std::string errorLog;
    errorLog.resize(maxLength);
    glGetShaderInfoLog(shader, maxLength, &maxLength, &errorLog[0]);

    glDeleteShader(shader); // Don't leak the shader.

    // Provide the infolog in whatever manor you deem best.
    return std::unexpected{ errorLog };
  }
  return shader;
}

inline std::expected<GLuint, std::string>
link(GLuint vs, GLuint fs)
{
  GLuint program = glCreateProgram();

  // Attach shaders as necessary.
  glAttachShader(program, vs);
  glAttachShader(program, fs);

  // Link the program.
  glLinkProgram(program);

  GLint isLinked = 0;
  glGetProgramiv(program, GL_LINK_STATUS, &isLinked);
  if (isLinked == GL_FALSE) {
    GLint maxLength = 0;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

    // The maxLength includes the NULL character
    std::string infoLog;
    infoLog.resize(maxLength);
    glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);

    // The program is useless now. So delete it.
    glDeleteProgram(program);

    // Provide the infolog in whatever manner you deem best.
    return std::unexpected{ infoLog };
  }
  return program;
}

template<typename T>
concept Float3 = sizeof(T) == sizeof(float) * 3;
template<typename T>
concept Mat3 = sizeof(T) == sizeof(float) * 9;
template<typename T>
concept Mat4 = sizeof(T) == sizeof(float) * 16;

struct UniformVariable
{
  uint32_t Location;
  void SetInt(int value) { glUniform1i(Location, value); }
  template<Float3 T>
  void SetFloat3(const T& t)
  {
    glUniform3fv(Location, 1, (const float*)&t);
  }
  template<Mat3 T>
  void SetMat3(const T& t)
  {
    glUniformMatrix3fv(Location, 1, GL_FALSE, (const float*)&t);
  }
  template<Mat4 T>
  void SetMat4(const T& t)
  {
    glUniformMatrix4fv(Location, 1, GL_FALSE, (const float*)&t);
  }
};

class ShaderProgram
{
  uint32_t program_ = 0;

  ShaderProgram(uint32_t program)
    : program_(program)
  {
  }

public:
  ~ShaderProgram() { glDeleteProgram(program_); }
  static std::expected<std::shared_ptr<ShaderProgram>, std::string> Create(
    std::span<std::u8string_view> vs_srcs,
    std::span<std::u8string_view> fs_srcs)
  {
    auto vs = compile(GL_VERTEX_SHADER, vs_srcs);
    if (!vs) {
      return std::unexpected{ std::string("vs: ") + vs.error() };
    }
    auto fs = compile(GL_FRAGMENT_SHADER, fs_srcs);
    if (!fs) {
      return std::unexpected{ std::string("fs: ") + fs.error() };
    }

    auto program = link(*vs, *fs);
    if (!program) {
      return std::unexpected{ program.error() };
    }

    return std::shared_ptr<ShaderProgram>(new ShaderProgram(*program));
  }
  static std::expected<std::shared_ptr<ShaderProgram>, std::string> Create(
    std::u8string_view vs,
    std::u8string_view fs)
  {
    std::u8string_view vss[] = { vs };
    std::u8string_view fss[] = { fs };
    return Create(vss, fss);
  }
  static std::expected<std::shared_ptr<ShaderProgram>, std::string>
  CreateFromPath(const std::filesystem::path& vs_path,
                 const std::filesystem::path& fs_path)
  {
    auto vs = grapho::StringFromPath(vs_path);
    auto fs = grapho::StringFromPath(fs_path);
    return Create(vs, fs);
  }
  void Use() { glUseProgram(program_); }
  void UnUse() { glUseProgram(0); }

  std::expected<uint32_t, std::string> Attribute(const char* name)
  {
    auto location = glGetAttribLocation(program_, name);
    if (location < 0) {
      return std::unexpected{ "glGetAttribLocation" };
    }
    return static_cast<uint32_t>(location);
  }

  std::expected<UniformVariable, std::string> Uniform(const std::string& name)
  {
    auto location = glGetUniformLocation(program_, name.c_str());
    if (location < 0) {
      return std::unexpected{ "glGetUniformLocation" };
    }
    return UniformVariable{ static_cast<uint32_t>(location) };
  }

  std::expected<uint32_t, std::string> UboBlockIndex(const char* name)
  {
    auto blockIndex = glGetUniformBlockIndex(program_, name);
    if (blockIndex < 0) {
      return std::unexpected{ "fail to glGetUniformBlockIndex" };
    }
    return blockIndex;
  }

  void UboBind(uint32_t blockIndex, uint32_t binding_point)
  {
    glUniformBlockBinding(program_, blockIndex, binding_point);
  }
};

//
//   // name
//   std::expected<void, std::string> _SetUniformMatrix(const char* name,
//                                                      const float m[16])
//   {
//     auto location = glGetUniformLocation(program_, name);
//     if (location < 0) {
//       return std::unexpected{ "fail to glGetUniformLocation" };
//     }
//     glUniformMatrix4fv(location, 1, GL_FALSE, m);
//     return {};
//   }
//
//   template<Mat4 T>
//   std::expected<void, std::string> SetUniformMatrix(const char* name,
//                                                     const T& t)
//   {
//     return _SetUniformMatrix(name, (float*)&t);
//   }
//
//   std::expected<void, std::string> SetUniformFloat(const char* name,
//                                                    float value)
//   {
//     auto location = glGetUniformLocation(program_, name);
//     if (location < 0) {
//       return std::unexpected{ "fail to glGetUniformLocation" };
//     }
//     glUniform1f(location, value);
//     return {};
//   }
//
//   std::expected<void, std::string> SetUniformFloat3(const char* name,
//                                                     const float value[3])
//   {
//     auto location = glGetUniformLocation(program_, name);
//     if (location < 0) {
//       return std::unexpected{ "fail to glGetUniformLocation" };
//     }
//     glUniform3f(location, value[0], value[1], value[2]);
//     return {};
//   }
//
//   std::expected<void, std::string> SetUniformFloat4(const char* name,
//                                                     const float value[4])
//   {
//     auto location = glGetUniformLocation(program_, name);
//     if (location < 0) {
//       return std::unexpected{ "fail to glGetUniformLocation" };
//     }
//     glUniform4f(location, value[0], value[1], value[2], value[3]);
//     return {};
//   }
//
//
// };

}
