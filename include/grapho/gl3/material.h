#pragma once
#include "shader.h"
#include "texture.h"

namespace grapho {
namespace gl3 {

struct TextureSlot
{
  uint32_t Unit;
  std::shared_ptr<Texture> m_texture;
  void Activate()
  {
    if (m_texture) {
      m_texture->Activate(Unit);
    } else {
      Texture::Deactivate(Unit);
    }
  }
};

struct Material
{
  std::shared_ptr<ShaderProgram> Shader;
  std::list<TextureSlot> Textures;

  struct EnvVars
  {
    DirectX::XMFLOAT4X4 projection;
    DirectX::XMFLOAT4X4 view;
    DirectX::XMFLOAT4 lightPositions[4];
    DirectX::XMFLOAT4 lightColors[4];
    DirectX::XMFLOAT4 camPos;
  };

  struct DrawVars
  {
    DirectX::XMFLOAT4X4 model;
    DirectX::XMFLOAT4 color;
    DirectX::XMFLOAT4 cutoff;
    DirectX::XMFLOAT4X4 normalMatrix;

    void CalcNormalMatrix()
    {
      auto m = DirectX::XMLoadFloat4x4(&model);
      DirectX::XMVECTOR det;
      auto ti = DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(&det, m));
      // DirectX::XMFLOAT3X3 mat3;
      DirectX::XMStoreFloat4x4(&normalMatrix, ti);
      // normalMatrix._41 = 0;
      // normalMatrix._42 = 0;
      // normalMatrix._43 = 0;
      // return mat3;
    }
  };

  void Activate(uint32_t envUbo = 0, uint32_t modelUbo = 1)
  {
    Shader->Use();
    Shader->UboBind(0, envUbo);
    Shader->UboBind(1, modelUbo);
    for (auto& slot : Textures) {
      slot.Activate();
    }
  }
};

}
}
