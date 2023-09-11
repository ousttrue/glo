#pragma once
#include "dxmath_stub.h"
#include <cmath>
#include <memory>
#include <string>
#include <vector>

namespace grapho {

enum class ValueType
{
  Float,
  Double,
  Int8,
  Int16,
  Int32,
  UInt8,
  UInt16,
  UInt32,
};

struct VertexId
{
  uint32_t AttributeLocation;
  uint32_t Slot;
  std::string SemanticName;
  uint32_t SemanticIndex;
};

struct VertexLayout
{
  VertexId Id;
  ValueType Type;
  uint32_t Count;
  uint32_t Offset;
  uint32_t Stride;
  uint32_t Divisor = 0;
};

enum class DrawMode
{
  Triangles,
  TriangleStrip,
};

struct VertexBuffer
{
  std::vector<uint8_t> Bytes;
  uint32_t Count = 0;
  template<typename T>
  void Assign(const std::vector<T>& values)
  {
    Count = values.size();
    Bytes.assign((const uint8_t*)values.data(),
                 (const uint8_t*)(values.data() + Count));
  }
  template<typename T, size_t N>
  void Assign(const T (&values)[N])
  {
    Count = N;
    Bytes.assign((const uint8_t*)values, (const uint8_t*)(values + Count));
  }
  uint32_t Size() const { return Bytes.size(); }
  const uint8_t* Data() const { return Bytes.data(); }
  uint32_t Stride() const { return Bytes.size() / Count; }
};

struct Mesh
{
  DrawMode Mode = DrawMode::Triangles;
  std::vector<grapho::VertexLayout> Layouts;
  VertexBuffer Vertices;
  VertexBuffer Indices;
  uint32_t DrawCount() const
  {
    return Indices.Size() ? Indices.Count : Vertices.Count;
  }
};

}
