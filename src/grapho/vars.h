#pragma once
#include "vertexlayout.h"

namespace grapho {

struct WorldVars
{
  XMFLOAT4 lightPositions[4];
  XMFLOAT4 lightColors[4];
  XMFLOAT4 camPos;
};

struct LocalVars
{
  XMFLOAT4X4 model;
  XMFLOAT4 color;
  XMFLOAT4 cutoff;
  XMFLOAT4X4 normalMatrix;
  XMFLOAT3 emissiveColor;
  XMFLOAT3X3 normalMatrix3() const;
  XMFLOAT3X3 uvTransform() const;
  void CalcNormalMatrix();
};

}
