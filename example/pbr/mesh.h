#pragma once
#include <DirectXMath.h>
#include <grapho/vertexlayout.h>
#include <memory>
#include <stdint.h>
#include <vector>

struct Mesh
{
  struct Vertex
  {
    DirectX::XMFLOAT3 Position;
    DirectX::XMFLOAT3 Normal;
    DirectX::XMFLOAT2 Uv;
  };
  grapho::VertexLayout Layouts[3]{
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
  static std::shared_ptr<Mesh> Sphere();
  static std::shared_ptr<Mesh> Cube();
};

