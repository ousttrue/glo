#pragma once
#include "../fileutil.h"
#include <expected>
#include <fstream>
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
concept Float4 = sizeof(T) == sizeof(float) * 4;
template<typename T>
concept Mat3 = sizeof(T) == sizeof(float) * 9;
template<typename T>
concept Mat4 = sizeof(T) == sizeof(float) * 16;

struct UniformVariable
{
  uint32_t Location;
  std::string Name;
  GLenum Type;

  void SetInt(int value) { glUniform1i(Location, value); }

  void SetFloat(float value) { glUniform1f(Location, value); }

  // 12 byte
  template<Float3 T>
  void Set(const T& t)
  {
    glUniform3fv(Location, 1, (const float*)&t);
  }
  // 16 byte
  template<Float4 T>
  void Set(const T& t)
  {
    glUniform4fv(Location, 1, (const float*)&t);
  }
  // 36 byte
  template<Mat3 T>
  void Set(const T& t)
  {
    glUniformMatrix3fv(Location, 1, GL_FALSE, (const float*)&t);
  }
  // 64 byte
  template<Mat4 T>
  void Set(const T& t)
  {
    glUniformMatrix4fv(Location, 1, GL_FALSE, (const float*)&t);
  }
};

inline void
DebugWrite(const std::string& path, std::span<std::u8string_view> srcs)
{
#ifndef NDEBUG
  std::stringstream ss;
  for (auto src : srcs) {
    ss << std::string_view((const char*)src.data(), src.size());
  }
  std::ofstream os(path);
  os << ss.str();
#endif
}

class ShaderProgram
{
  uint32_t program_ = 0;

  ShaderProgram(uint32_t program)
    : program_(program)
  {
    // https://stackoverflow.com/questions/440144/in-opengl-is-there-a-way-to-get-a-list-of-all-uniforms-attribs-used-by-a-shade
    int count;
    glGetProgramiv(program_, GL_ACTIVE_UNIFORMS, &count);
    // printf("Active Uniforms: %d\n", count);
    Uniforms.reserve(count);

    for (GLuint i = 0; i < count; i++) {
      // maximum name length
      const GLsizei bufSize = 64;
      // variable name in GLSL
      GLchar name[bufSize];
      // size of the variable
      GLint size;
      // type of the variable (float, vec3 or mat4, etc)
      GLenum type;
      // name length
      GLsizei length;
      glGetActiveUniform(program_, i, bufSize, &length, &size, &type, name);

      Uniforms.push_back({
        .Location = i,
        .Name = std::string{ name, length },
        .Type = type,
      });
      //
      // printf("Uniform #%d Type: %u Name: %s\n", i, type, name);
    }
  }

public:
  std::vector<UniformVariable> Uniforms;
  ~ShaderProgram() { glDeleteProgram(program_); }
  static std::expected<std::shared_ptr<ShaderProgram>, std::string> Create(
    std::span<std::u8string_view> vs_srcs,
    std::span<std::u8string_view> fs_srcs)
  {
    auto vs = compile(GL_VERTEX_SHADER, vs_srcs);
    if (!vs) {
      DebugWrite("debug.vert", vs_srcs);
      return std::unexpected{ std::string("vs: ") + vs.error() };
    }
    auto fs = compile(GL_FRAGMENT_SHADER, fs_srcs);
    if (!fs) {
      DebugWrite("debug.frag", fs_srcs);
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
  static std::expected<std::shared_ptr<ShaderProgram>, std::string> Create(
    std::string_view vs,
    std::string_view fs)
  {
    std::u8string_view vss[] = { { (const char8_t*)vs.data(),
                                   (const char8_t*)vs.data() + vs.size() } };
    std::u8string_view fss[] = { { (const char8_t*)fs.data(),
                                   (const char8_t*)fs.data() + fs.size() } };
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

  std::optional<uint32_t> Attribute(const char* name)
  {
    auto location = glGetAttribLocation(program_, name);
    if (location < 0) {
      return std::nullopt;
    }
    return static_cast<uint32_t>(location);
  }

  std::optional<UniformVariable> Uniform(const std::string& name)
  {
    auto location = glGetUniformLocation(program_, name.c_str());
    if (location < 0) {
      return std::nullopt;
    }
    return UniformVariable{ static_cast<uint32_t>(location) };
  }

  template<typename T>
  void SetUniform(const std::string& name, const T& value)
  {
    if (auto var = Uniform(name)) {
      var->Set(value);
    }
  }

  void SetUniform(const std::string& name, int value)
  {
    if (auto var = Uniform(name)) {
      var->SetInt(value);
    }
  }

  void SetUniform(const std::string& name, float value)
  {
    if (auto var = Uniform(name)) {
      var->SetFloat(value);
    }
  }

  std::optional<uint32_t> UboBlockIndex(const char* name)
  {
    auto blockIndex = glGetUniformBlockIndex(program_, name);
    if (blockIndex < 0) {
      return std::nullopt;
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
