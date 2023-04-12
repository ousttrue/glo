#pragma once
#include <expected>
#include <string>
#include <vector>

namespace grapho {
const int CUBE_INDEX_COUNT = 36;

enum class ValueType
{
  Float,
};

struct VertexId
{
  std::string semantic_name;
  uint32_t semantic_index;
};

struct VertexLayout
{
  VertexId id;
  ValueType type;
  uint32_t count;
  uint32_t offset;
  uint32_t stride;
  uint32_t divisor = 0;
};

struct XY
{
  float x;
  float y;
};

struct XYZ
{
  float x;
  float y;
  float z;

  XYZ operator+(const XYZ& rhs) const
  {
    return {
      x + rhs.x,
      y + rhs.y,
      z + rhs.z,
    };
  }
  XYZ& operator+=(const XYZ& rhs)
  {
    x += rhs.x;
    y += rhs.y;
    z += rhs.z;
    return *this;
  }
  XYZ operator*(float s) const { return { x * s, y * s, z * s }; }
};

struct Vertex
{
  XYZ position;
  XY barycentric;
};
static_assert(sizeof(Vertex) == sizeof(float) * 5, "sizeof Vertex");

struct Mesh
{
  std::vector<Vertex> vertices;
  std::vector<uint32_t> indices;
  std::vector<VertexLayout> layouts;

  Mesh() {}
  ~Mesh() {}
};

struct XYZW
{
  float x;
  float y;
  float z;
  float w;
};

struct RGBA
{
  float r;
  float g;
  float b;
  float a;
};

struct Instance
{
  XYZW row0;
  XYZW row1;
  XYZW row2;
  XYZW row3;
};

Mesh
Cube(bool isCCW, bool isStereo);

struct LineVertex
{
  XYZ position;
  RGBA color;
};
static_assert(sizeof(LineVertex) == 28, "sizeof LineVertex");

void
PushGrid(std::vector<LineVertex>& lines,
         float interval = 1.0f,
         int half_count = 5);

}
