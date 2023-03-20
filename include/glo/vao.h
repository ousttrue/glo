#pragma once
#include "mesh.h"
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

public:
  Vbo(uint32_t vbo);
  ~Vbo();
  static std::shared_ptr<Vbo> Create(uint32_t size, const void* data = nullptr);

  template<ArrayType T>
  static std::shared_ptr<Vbo> Create(const T& array)
  {
    return Create(sizeof(array), array);
  }

  void Bind();
  void Unbind();
  void Upload(uint32_t size, const void* data);
};

class Ibo
{
  uint32_t ibo_ = 0;

public:
  uint32_t valuetype_ = 0;
  Ibo(uint32_t ibo, uint32_t valuetype);
  ~Ibo();
  static std::shared_ptr<Ibo> Create(uint32_t size,
                                     const void* data,
                                     uint32_t valuetype);
  void Bind();
  void Unbind();
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
      const std::shared_ptr<Ibo>& ibo);
  ~Vao();
  static std::shared_ptr<Vao> Create(std::span<VertexLayout> layouts,
                                     std::span<VertexSlot> slots,
                                     const std::shared_ptr<Ibo>& ibo = {});
  void Bind();
  void Unbind();
  void Draw(uint32_t mode, uint32_t count, uint32_t offset = 0);
  void DrawInstance(uint32_t primcount, uint32_t count, uint32_t offset = 0);
};

}
