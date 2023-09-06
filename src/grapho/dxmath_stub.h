#pragma once
#if !defined(DIRECTX_MATH_VERSION)
#include <xmmintrin.h>

namespace DirectX {

struct XMFLOAT2
{
  float x, y, z;
};

struct XMFLOAT3
{
  float x, y, z;
};

struct XMFLOAT4
{
  float x, y, z, w;
};

struct XMFLOAT3X3
{
  union
  {
    struct
    {
      float _11, _12, _13;
      float _21, _22, _23;
      float _31, _32, _33;
    };
    float m[3][3];
  };
};

struct XMFLOAT4X4
{
  union
  {
    struct
    {
      float _11, _12, _13, _14;
      float _21, _22, _23, _24;
      float _31, _32, _33, _34;
      float _41, _42, _43, _44;
    };
    float m[4][4];
  };
};

using XMVECTOR = __m128;

__declspec(align(16)) struct XMMATRIX
{
  XMVECTOR r[4];
};

}
#endif
