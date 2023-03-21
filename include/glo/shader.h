#pragma once
#include <expected>
#include <functional>
#include <memory>
#include <optional>
#include <span>
#include <stdint.h>
#include <string_view>

namespace glo {

inline std::expected<GLuint, std::string>
compile(GLenum shaderType, std::span<std::string_view> srcs)
{
  auto shader = glCreateShader(shaderType);

  std::vector<const GLchar*> string;
  std::vector<GLint> length;
  for (auto src : srcs) {
    string.push_back(src.data());
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
concept Mat4 = sizeof(T) == 4 * 16;

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
    std::span<std::string_view> vs_srcs,
    std::span<std::string_view> fs_srcs)
  {
    auto vs = compile(GL_VERTEX_SHADER, vs_srcs);
    if (!vs) {
      return std::unexpected{ vs.error() };
    }
    auto fs = compile(GL_FRAGMENT_SHADER, fs_srcs);
    if (!fs) {
      return std::unexpected{ fs.error() };
    }

    auto program = link(*vs, *fs);
    if (!program) {
      return std::unexpected{ program.error() };
    }

    return std::shared_ptr<ShaderProgram>(new ShaderProgram(*program));
  }
  static std::expected<std::shared_ptr<ShaderProgram>, std::string> Create(
    std::string_view vs,
    std::string_view fs)
  {
    std::string_view vss[] = { vs };
    std::string_view fss[] = { fs };
    return Create(vss, fss);
  }
  void Bind() { glUseProgram(program_); }
  void Unbind() { glUseProgram(0); }

  std::expected<uint32_t, std::string> AttributeLocation(const char* name)
  {
    auto location = glGetAttribLocation(program_, name);
    if (location < 0) {
      return std::unexpected{ "glGetAttribLocation" };
    }
    return static_cast<uint32_t>(location);
  }
  std::expected<uint32_t, std::string> UniformLocation(const char* name)
  {
    auto location = glGetUniformLocation(program_, name);
    if (location < 0) {
      return std::unexpected{ "glGetUniformLocation" };
    }
    return static_cast<uint32_t>(location);
  }

  // location
  void _SetUniformMatrix(uint32_t location, const float m[16])
  {
    glUniformMatrix4fv(location, 1, GL_FALSE, m);
  }

  template<Mat4 T>
  void SetUniformMatrix(uint32_t location, const T& t)
  {
    return _SetUniformMatrix(location, (float*)&t);
  }

  // name
  std::expected<void, std::string> _SetUniformMatrix(const char* name,
                                                     const float m[16])
  {
    auto location = glGetUniformLocation(program_, name);
    if (location < 0) {
      return std::unexpected{ "fail to glGetUniformLocation" };
    }
    glUniformMatrix4fv(location, 1, GL_FALSE, m);
    return {};
  }

  template<Mat4 T>
  std::expected<void, std::string> SetUniformMatrix(const char* name,
                                                    const T& t)
  {
    return _SetUniformMatrix(name, (float*)&t);
  }
};

}
