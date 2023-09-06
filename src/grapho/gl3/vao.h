#pragma once
#include "../vertexlayout.h"
#include <assert.h>
#include <memory>
#include <span>
#include <stdexcept>
#include <stdint.h>
#include <vector>

namespace grapho::gl3 {

inline std::expected<uint32_t, std::string>
GLType(ValueType type)
{
  switch (type) {
    case ValueType::Float:
      return GL_FLOAT;
    case ValueType::Double:
      return GL_DOUBLE;

    case ValueType::Int8:
      return GL_BYTE;
    case ValueType::Int16:
      return GL_SHORT;
    case ValueType::Int32:
      return GL_INT;

    case ValueType::UInt8:
      return GL_UNSIGNED_BYTE;
    case ValueType::UInt16:
      return GL_UNSIGNED_SHORT;
    case ValueType::UInt32:
      return GL_UNSIGNED_INT;
  }

  return {};
}

inline std::expected<uint32_t, std::string>
GLIndexTypeFromStride(uint32_t stride)
{
  switch (stride) {
    case 1:
      return GL_UNSIGNED_BYTE;

    case 2:
      return GL_UNSIGNED_SHORT;

    case 4:
      return GL_UNSIGNED_INT;

    default:
      return std::unexpected{ "invalid index stride" };
  }
}

inline std::expected<uint32_t, std::string>
GLMode(grapho::DrawMode mode)
{
  switch (mode) {
    case DrawMode::Triangles:
      return GL_TRIANGLES;
    case DrawMode::TriangleStrip:
      return GL_TRIANGLE_STRIP;
    default:
      return std::unexpected{ "unknown GLType" };
  }
}

template<typename T>
concept ArrayType = std::is_array<T>::value == true;

class Vbo
{
  uint32_t vbo_ = 0;

  Vbo(uint32_t vbo)
    : vbo_(vbo)
  {
  }

public:
  ~Vbo() { glDeleteBuffers(1, &vbo_); }
  static std::shared_ptr<Vbo> Create(uint32_t size, const void* data)
  {
    GLuint vbo;
    glGenBuffers(1, &vbo);
    auto ptr = std::shared_ptr<Vbo>(new Vbo(vbo));
    ptr->Bind();
    if (data) {
      glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
    } else {
      glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
    }
    ptr->Unbind();
    return ptr;
  }
  template<ArrayType T>
  static std::shared_ptr<Vbo> Create(const T& array)
  {
    return Create(sizeof(array), array);
  }
  template<typename T>
  static std::shared_ptr<Vbo> Create(const std::vector<T>& values)
  {
    return Create(sizeof(T) * values.size(), values.data());
  }

  void Bind() { glBindBuffer(GL_ARRAY_BUFFER, vbo_); }
  void Unbind() { glBindBuffer(GL_ARRAY_BUFFER, 0); }
  void Upload(uint32_t size, const void* data)
  {
    Bind();
    glBufferSubData(GL_ARRAY_BUFFER, 0, size, data);
    Unbind();
  }
};

class Ibo
{
  uint32_t ibo_ = 0;

public:
  uint32_t valuetype_ = 0;
  Ibo(uint32_t ibo, uint32_t valuetype)
    : ibo_(ibo)
    , valuetype_(valuetype)
  {
  }
  ~Ibo() { glDeleteBuffers(1, &ibo_); }
  static std::shared_ptr<Ibo> Create(uint32_t size,
                                     const void* data,
                                     uint32_t valuetype)
  {
    GLuint ibo;
    glGenBuffers(1, &ibo);
    auto ptr = std::shared_ptr<Ibo>(new Ibo(ibo, valuetype));
    ptr->Bind();
    if (data) {
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
    } else {
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
    }
    ptr->Unbind();
    return ptr;
  }
  template<typename T>
  static std::shared_ptr<Ibo> Create(const std::vector<T>& values)
  {
    switch (sizeof(T)) {
      case 1:
        return Create(values.size(), values.data(), GL_UNSIGNED_BYTE);

      case 2:
        return Create(values.size() * 2, values.data(), GL_UNSIGNED_SHORT);

      case 4:
        return Create(values.size() * 4, values.data(), GL_UNSIGNED_INT);
    }
    return {};
  }
  void Bind() { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_); }
  void Unbind() { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); }
};

struct Vao
{
  uint32_t vao_ = 0;
  std::vector<VertexLayout> layouts_;
  std::vector<std::shared_ptr<Vbo>> slots_;
  std::shared_ptr<Ibo> ibo_;

  Vao(uint32_t vao,
      std::span<VertexLayout> layouts,
      std::span<std::shared_ptr<Vbo>> slots,
      const std::shared_ptr<Ibo>& ibo)
    : vao_(vao)
    , layouts_(layouts.begin(), layouts.end())
    , slots_(slots.begin(), slots.end())
    , ibo_(ibo)
  {
    Bind();
    if (ibo_) {
      ibo_->Bind();
    }
    for (int i = 0; i < layouts.size(); ++i) {
      auto& layout = layouts[i];
      glEnableVertexAttribArray(layout.Id.AttributeLocation);
      slots[layout.Id.Slot]->Bind();
      glVertexAttribPointer(
        layout.Id.AttributeLocation,
        layout.Count,
        *GLType(layout.Type),
        GL_FALSE,
        layout.Stride,
        reinterpret_cast<void*>(static_cast<uint64_t>(layout.Offset)));
      if (layout.Divisor) {
        // auto a = glVertexAttribDivisor;
        glVertexAttribDivisor(layout.Id.AttributeLocation, layout.Divisor);
      }
    }
    Unbind();
    if (ibo_) {
      ibo_->Unbind();
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);
  }
  ~Vao() { glDeleteVertexArrays(1, &vao_); }
  static std::shared_ptr<Vao> Create(std::span<VertexLayout> layouts,
                                     std::span<std::shared_ptr<Vbo>> slots,
                                     const std::shared_ptr<Ibo>& ibo = {})
  {
    GLuint vao;
    glGenVertexArrays(1, &vao);
    auto ptr = std::shared_ptr<Vao>(new Vao(vao, layouts, slots, ibo));
    return ptr;
  }
  static std::shared_ptr<Vao> Create(const std::shared_ptr<Mesh>& mesh)
  {
    std::shared_ptr<Vbo> slots[1] = { Vbo::Create(mesh->Vertices.Size(),
                                                  mesh->Vertices.Data()) };
    std::shared_ptr<Ibo> ibo;
    if (mesh->Indices.Size()) {
      ibo = Ibo::Create(mesh->Indices.Size(),
                        mesh->Indices.Data(),
                        *GLIndexTypeFromStride(mesh->Indices.Stride()));
    }
    return Create(mesh->Layouts, slots, ibo);
  }
  void Bind() { glBindVertexArray(vao_); }
  void Unbind() { glBindVertexArray(0); }
  void Draw(uint32_t mode, uint32_t count, uint32_t offsetBytes = 0)
  {
    Bind();
    if (ibo_) {
      glDrawElements(
        mode,
        count,
        ibo_->valuetype_,
        reinterpret_cast<void*>(static_cast<uint64_t>(offsetBytes)));
    } else {
      glDrawArrays(mode, offsetBytes, count);
    }
    Unbind();
  }
  void DrawInstance(uint32_t primcount,
                    uint32_t count,
                    uint32_t offsetBytes = 0)
  {
    Bind();
    if (ibo_) {
      glDrawElementsInstanced(
        GL_TRIANGLES,
        count,
        ibo_->valuetype_,
        reinterpret_cast<void*>(static_cast<uint64_t>(offsetBytes)),
        primcount);
    } else {
      throw std::runtime_error("not implemented");
    }
    Unbind();
  }
};
}
