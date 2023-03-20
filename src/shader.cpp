#include <GL/glew.h>
#include <glo/shader.h>

static GLuint
compile(const std::function<void(std::string_view)>& onError,
        GLenum shaderType,
        std::span<std::string_view> srcs)
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
    std::vector<GLchar> errorLog(maxLength);
    glGetShaderInfoLog(shader, maxLength, &maxLength, &errorLog[0]);

    glDeleteShader(shader); // Don't leak the shader.

    // Provide the infolog in whatever manor you deem best.
    onError({ errorLog.begin(), errorLog.end() });
    // Exit with failure.
    return 0;
  }
  return shader;
}

static GLuint
link(const std::function<void(std::string_view)>& onError, GLuint vs, GLuint fs)
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
    std::vector<GLchar> infoLog(maxLength);
    glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);

    // The program is useless now. So delete it.
    glDeleteProgram(program);

    // Provide the infolog in whatever manner you deem best.
    onError({ infoLog.begin(), infoLog.end() });
    // Exit with failure.
    return {};
  }
  return program;
}

namespace glo {

ShaderProgram::ShaderProgram(uint32_t program)
  : program_(program)
{
}

ShaderProgram::~ShaderProgram() {}

std::shared_ptr<ShaderProgram>
ShaderProgram::Create(const std::function<void(std::string_view)>& onError,
                      std::span<std::string_view> vs_srcs,
                      std::span<std::string_view> fs_srcs)
{

  auto vs = compile(onError, GL_VERTEX_SHADER, vs_srcs);
  if (!vs) {
    return {};
  }
  auto fs = compile(onError, GL_FRAGMENT_SHADER, fs_srcs);
  if (!fs) {
    return {};
  }

  auto program = link(onError, vs, fs);
  if (!program) {
    return {};
  }

  return std::shared_ptr<ShaderProgram>(new ShaderProgram(program));
}

void
ShaderProgram::Bind()
{
  glUseProgram(program_);
}
void
ShaderProgram::Unbind()
{
  glUseProgram(0);
}

std::optional<uint32_t>
ShaderProgram::AttributeLocation(const char* name)
{
  auto location = glGetAttribLocation(program_, name);
  if (location < 0) {
    return {};
  }
  return static_cast<uint32_t>(location);
}

std::optional<uint32_t>
ShaderProgram::UniformLocation(const char* name)
{
  auto location = glGetUniformLocation(program_, name);
  if (location < 0) {
    return {};
  }
  return static_cast<uint32_t>(location);
}

void
ShaderProgram::_SetUniformMatrix(const ErrorHandler& onError,
                                 uint32_t location,
                                 const float m[16])
{
  glUniformMatrix4fv(location, 1, GL_FALSE, m);
}

void
ShaderProgram::_SetUniformMatrix(const ErrorHandler& onError,
                                 const char* name,
                                 const float m[16])
{
  auto location = glGetUniformLocation(program_, name);
  if (location < 0) {
    onError("fail to glGetUniformLocation");
    return;
  }
  glUniformMatrix4fv(location, 1, GL_FALSE, m);
}

}
