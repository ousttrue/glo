#include <DirectXMath.h>

#include "cuberenderer.h"

namespace grapho {
namespace gl3 {

CubeRenderer::CubeRenderer()
{
  auto cube = grapho::mesh::Cube();
  m_cube = grapho::gl3::Vao::Create(cube);
  m_cubeDrawCount = cube->DrawCount();
  m_mode = *grapho::gl3::GLMode(cube->Mode);

  DirectX::XMStoreFloat4x4(
    &m_captureProjection,
    DirectX::XMMatrixPerspectiveFovRH(
      DirectX::XMConvertToRadians(90.0f), 1.0f, 0.1f, 10.0f));

  DirectX::XMStoreFloat4x4(
    &m_captureViews[0],
    DirectX::XMMatrixLookAtRH(DirectX::XMVectorZero(),
                              DirectX::XMVectorSet(1.0f, 0, 0, 0),
                              DirectX::XMVectorSet(0, -1.0f, 0, 0)));
  DirectX::XMStoreFloat4x4(
    &m_captureViews[1],
    DirectX::XMMatrixLookAtRH(DirectX::XMVectorZero(),
                              DirectX::XMVectorSet(-1.0f, 0, 0, 0),
                              DirectX::XMVectorSet(0, -1.0f, 0, 0)));
  DirectX::XMStoreFloat4x4(
    &m_captureViews[2],
    DirectX::XMMatrixLookAtRH(DirectX::XMVectorZero(),
                              DirectX::XMVectorSet(0, 1.0f, 0, 0),
                              DirectX::XMVectorSet(0, 0, 1.0f, 0)));
  DirectX::XMStoreFloat4x4(
    &m_captureViews[3],
    DirectX::XMMatrixLookAtRH(DirectX::XMVectorZero(),
                              DirectX::XMVectorSet(0, -1.0f, 0, 0),
                              DirectX::XMVectorSet(0, 0, -1.0f, 0)));
  DirectX::XMStoreFloat4x4(
    &m_captureViews[4],
    DirectX::XMMatrixLookAtRH(DirectX::XMVectorZero(),
                              DirectX::XMVectorSet(0, 0, 1.0f, 0),
                              DirectX::XMVectorSet(0, -1.0f, 0, 0)));
  DirectX::XMStoreFloat4x4(
    &m_captureViews[5],
    DirectX::XMMatrixLookAtRH(DirectX::XMVectorZero(),
                              DirectX::XMVectorSet(0, 0, -1.0f, 0),
                              DirectX::XMVectorSet(0, -1.0f, 0, 0)));
}

} // namespace
} // namespace
