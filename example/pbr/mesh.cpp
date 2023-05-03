#include "mesh.h"
#include <cmath>

std::shared_ptr<Mesh>
Mesh::Sphere()
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
