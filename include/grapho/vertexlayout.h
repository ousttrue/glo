#pragma once
#include <DirectXMath.h>
#include <cmath>
#include <expected>
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

struct Mesh
{
  struct Vertex
  {
    DirectX::XMFLOAT3 Position;
    DirectX::XMFLOAT3 Normal;
    DirectX::XMFLOAT2 Uv;
  };
  DrawMode Mode = DrawMode::Triangles;
  std::vector<grapho::VertexLayout> Layouts{
    {
      .Id = {
       .AttributeLocation=0,
       .Slot=0,
      },
      .Type = grapho::ValueType::Float,
      .Count = 3,
      .Offset = offsetof(Vertex, Position),
      .Stride = sizeof(Vertex),
    },
    {
      .Id = {
       .AttributeLocation=1,
       .Slot=0,
      },
      .Type = grapho::ValueType::Float,
      .Count = 3,
      .Offset = offsetof(Vertex, Normal),
      .Stride = sizeof(Vertex),
    },
    {
      .Id = {
       .AttributeLocation=2,
       .Slot=0,
      },
      .Type = grapho::ValueType::Float,
      .Count = 2,
      .Offset = offsetof(Vertex, Uv),
      .Stride = sizeof(Vertex),
    },
  };
  std::vector<Vertex> Vertices;
  std::vector<unsigned int> Indices;

  static std::shared_ptr<Mesh> Sphere()
  {
    std::vector<DirectX::XMFLOAT3> positions;
    std::vector<DirectX::XMFLOAT2> uv;
    std::vector<DirectX::XMFLOAT3> normals;
    const unsigned int X_SEGMENTS = 64;
    const unsigned int Y_SEGMENTS = 64;
    const float PI = 3.14159265359f;
    for (unsigned int x = 0; x <= X_SEGMENTS; ++x) {
      for (unsigned int y = 0; y <= Y_SEGMENTS; ++y) {
        float xSegment = (float)x / (float)X_SEGMENTS;
        float ySegment = (float)y / (float)Y_SEGMENTS;
        float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
        float yPos = std::cos(ySegment * PI);
        float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
        positions.push_back({ xPos, yPos, zPos });
        uv.push_back({ xSegment, ySegment });
        normals.push_back({ xPos, yPos, zPos });
      }
    }

    auto ptr = std::make_shared<Mesh>();
    bool oddRow = false;
    for (unsigned int y = 0; y < Y_SEGMENTS; ++y) {
      if (!oddRow) // even rows: y == 0, y == 2; and so on
      {
        for (unsigned int x = 0; x <= X_SEGMENTS; ++x) {
          ptr->Indices.push_back(y * (X_SEGMENTS + 1) + x);
          ptr->Indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
        }
      } else {
        for (int x = X_SEGMENTS; x >= 0; --x) {
          ptr->Indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
          ptr->Indices.push_back(y * (X_SEGMENTS + 1) + x);
        }
      }
      oddRow = !oddRow;
    }

    for (unsigned int i = 0; i < positions.size(); ++i) {
      ptr->Vertices.push_back({
        positions[i],
        normals[i],
        uv[i],
      });
    }

    return ptr;
  }

  static std::shared_ptr<Mesh> Cube()
  {
    std::vector<Mesh::Vertex> vertices{
      // back face
      { { -1.0f, -1.0f, -1.0f },
        { 0.0f, 0.0f, -1.0f },
        { 0.0f, 0.0f } }, // bottom-left
      { { 1.0f, 1.0f, -1.0f },
        { 0.0f, 0.0f, -1.0f },
        { 1.0f, 1.0f } }, // top-right
      { { 1.0f, -1.0f, -1.0f },
        { 0.0f, 0.0f, -1.0f },
        { 1.0f, 0.0f } }, // bottom-right
      { { 1.0f, 1.0f, -1.0f },
        { 0.0f, 0.0f, -1.0f },
        { 1.0f, 1.0f } }, // top-right
      { { -1.0f, -1.0f, -1.0f },
        { 0.0f, 0.0f, -1.0f },
        { 0.0f, 0.0f } }, // bottom-left
      { { -1.0f, 1.0f, -1.0f },
        { 0.0f, 0.0f, -1.0f },
        { 0.0f, 1.0f } }, // top-left
      // front face
      { { -1.0f, -1.0f, 1.0f },
        { 0.0f, 0.0f, 1.0f },
        { 0.0f, 0.0f } }, // bottom-left
      { { 1.0f, -1.0f, 1.0f },
        { 0.0f, 0.0f, 1.0f },
        { 1.0f, 0.0f } }, // bottom-right
      { { 1.0f, 1.0f, 1.0f },
        { 0.0f, 0.0f, 1.0f },
        { 1.0f, 1.0f } }, // top-right
      { { 1.0f, 1.0f, 1.0f },
        { 0.0f, 0.0f, 1.0f },
        { 1.0f, 1.0f } }, // top-right
      { { -1.0f, 1.0f, 1.0f },
        { 0.0f, 0.0f, 1.0f },
        { 0.0f, 1.0f } }, // top-left
      { { -1.0f, -1.0f, 1.0f },
        { 0.0f, 0.0f, 1.0f },
        { 0.0f, 0.0f } }, // bottom-left
      // left face
      { { -1.0f, 1.0f, 1.0f },
        { -1.0f, 0.0f, 0.0f },
        { 1.0f, 0.0f } }, // top-right
      { { -1.0f, 1.0f, -1.0f },
        { -1.0f, 0.0f, 0.0f },
        { 1.0f, 1.0f } }, // top-left
      { { -1.0f, -1.0f, -1.0f },
        { -1.0f, 0.0f, 0.0f },
        { 0.0f, 1.0f } }, // bottom-left
      { { -1.0f, -1.0f, -1.0f },
        { -1.0f, 0.0f, 0.0f },
        { 0.0f, 1.0f } }, // bottom-left
      { { -1.0f, -1.0f, 1.0f },
        { -1.0f, 0.0f, 0.0f },
        { 0.0f, 0.0f } }, // bottom-right
      { { -1.0f, 1.0f, 1.0f },
        { -1.0f, 0.0f, 0.0f },
        { 1.0f, 0.0f } }, // top-right
                          // right face
      { { 1.0f, 1.0f, 1.0f },
        { 1.0f, 0.0f, 0.0f },
        { 1.0f, 0.0f } }, // top-left
      { { 1.0f, -1.0f, -1.0f },
        { 1.0f, 0.0f, 0.0f },
        { 0.0f, 1.0f } }, // bottom-right
      { { 1.0f, 1.0f, -1.0f },
        { 1.0f, 0.0f, 0.0f },
        { 1.0f, 1.0f } }, // top-right
      { { 1.0f, -1.0f, -1.0f },
        { 1.0f, 0.0f, 0.0f },
        { 0.0f, 1.0f } }, // bottom-right
      { { 1.0f, 1.0f, 1.0f },
        { 1.0f, 0.0f, 0.0f },
        { 1.0f, 0.0f } }, // top-left
      { { 1.0f, -1.0f, 1.0f },
        { 1.0f, 0.0f, 0.0f },
        { 0.0f, 0.0f } }, // bottom-left
      // bottom face
      { { -1.0f, -1.0f, -1.0f },
        { 0.0f, -1.0f, 0.0f },
        { 0.0f, 1.0f } }, // top-right
      { { 1.0f, -1.0f, -1.0f },
        { 0.0f, -1.0f, 0.0f },
        { 1.0f, 1.0f } }, // top-left
      { { 1.0f, -1.0f, 1.0f },
        { 0.0f, -1.0f, 0.0f },
        { 1.0f, 0.0f } }, // bottom-left
      { { 1.0f, -1.0f, 1.0f },
        { 0.0f, -1.0f, 0.0f },
        { 1.0f, 0.0f } }, // bottom-left
      { { -1.0f, -1.0f, 1.0f },
        { 0.0f, -1.0f, 0.0f },
        { 0.0f, 0.0f } }, // bottom-right
      { { -1.0f, -1.0f, -1.0f },
        { 0.0f, -1.0f, 0.0f },
        { 0.0f, 1.0f } }, // top-right
      // top face
      { { -1.0f, 1.0f, -1.0f },
        { 0.0f, 1.0f, 0.0f },
        { 0.0f, 1.0f } }, // top-left
      { { 1.0f, 1.0f, 1.0f },
        { 0.0f, 1.0f, 0.0f },
        { 1.0f, 0.0f } }, // bottom-right
      { { 1.0f, 1.0f, -1.0f },
        { 0.0f, 1.0f, 0.0f },
        { 1.0f, 1.0f } }, // top-right
      { { 1.0f, 1.0f, 1.0f },
        { 0.0f, 1.0f, 0.0f },
        { 1.0f, 0.0f } }, // bottom-right
      { { -1.0f, 1.0f, -1.0f },
        { 0.0f, 1.0f, 0.0f },
        { 0.0f, 1.0f } }, // top-left
      { { -1.0f, 1.0f, 1.0f },
        { 0.0f, 1.0f, 0.0f },
        { 0.0f, 0.0f } } // bottom-left
    };
    auto ptr = std::make_shared<Mesh>();
    ptr->Vertices = vertices;
    return ptr;
  }
};
}
