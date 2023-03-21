#pragma once
#include "mesh.h"
#include <assert.h>
#include <memory>
#include <span>
#include <stdint.h>
#include <vector>

namespace glo {

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
  void Bind() { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_); }
  void Unbind() { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); }
};

struct VertexSlot
{
  uint32_t location;
  std::shared_ptr<Vbo> vbo;
};

class Vao
{
  uint32_t vao_ = 0;
  std::vector<VertexLayout> layouts_;
  std::vector<VertexSlot> slots_;
  std::shared_ptr<Ibo> ibo_;

public:
  Vao(uint32_t vao,
      std::span<VertexLayout> layouts,
      std::span<VertexSlot> slots,
      const std::shared_ptr<Ibo>& ibo)
    : vao_(vao)
    , layouts_(layouts.begin(), layouts.end())
    , slots_(slots.begin(), slots.end())
    , ibo_(ibo)
  {
    assert(layouts.size() == slots.size());
    Bind();
    if (ibo_) {
      ibo_->Bind();
    }
    for (int i = 0; i < layouts.size(); ++i) {
      auto& layout = layouts[i];
      auto& slot = slots[i];
      glEnableVertexAttribArray(slot.location);
      slot.vbo->Bind();
      glVertexAttribPointer(
        slot.location,
        layout.count,
        *GLType(layout.type),
        GL_FALSE,
        layout.stride,
        reinterpret_cast<void*>(static_cast<uint64_t>(layout.offset)));
      if (layout.divisor) {
        auto a = glVertexAttribDivisor;
        glVertexAttribDivisor(slot.location, layout.divisor);
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
                                     std::span<VertexSlot> slots,
                                     const std::shared_ptr<Ibo>& ibo = {})
  {
    GLuint vao;
    glGenVertexArrays(1, &vao);
    auto ptr = std::shared_ptr<Vao>(new Vao(vao, layouts, slots, ibo));
    return ptr;
  }
  void Bind() { glBindVertexArray(vao_); }
  void Unbind() { glBindVertexArray(0); }
  void Draw(uint32_t mode, uint32_t count, uint32_t offset = 0)
  {
    Bind();
    if (ibo_) {
      glDrawElements(mode,
                     count,
                     ibo_->valuetype_,
                     reinterpret_cast<void*>(static_cast<uint64_t>(offset)));
    } else {
      glDrawArrays(mode, offset, count);
    }
    Unbind();
  }
  void DrawInstance(uint32_t primcount, uint32_t count, uint32_t offset = 0)
  {
    Bind();
    if (ibo_) {
      glDrawElementsInstanced(
        GL_TRIANGLES,
        count,
        ibo_->valuetype_,
        reinterpret_cast<void*>(static_cast<uint64_t>(offset)),
        primcount);
    } else {
      throw std::runtime_error("not implemented");
    }
    Unbind();
  }
};

}