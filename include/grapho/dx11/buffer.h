#pragma once
#include <d3d11.h>
#include <winrt/base.h>

namespace grapho {
namespace dx11 {

inline winrt::com_ptr<ID3D11Buffer>
CreateIndexBuffer(const winrt::com_ptr<ID3D11Device>& device,
                  uint32_t size,
                  const void* data)
{
  D3D11_BUFFER_DESC index_buff_desc = {
    .ByteWidth = size,
    .Usage = D3D11_USAGE_DEFAULT,
    .BindFlags = D3D11_BIND_INDEX_BUFFER,
  };
  D3D11_SUBRESOURCE_DATA sr_data = {
    .pSysMem = data,
  };
  winrt::com_ptr<ID3D11Buffer> index_buffer;
  auto hr =
    device->CreateBuffer(&index_buff_desc, &sr_data, index_buffer.put());
  if (FAILED(hr)) {
    return {};
  }
  return index_buffer;
}

inline winrt::com_ptr<ID3D11Buffer>
CreateVertexBuffer(const winrt::com_ptr<ID3D11Device>& device,
                   uint32_t size,
                   const void* data)
{
  D3D11_BUFFER_DESC vertex_buff_desc = {
    .ByteWidth = size,
    .Usage = D3D11_USAGE_DEFAULT,
    .BindFlags = D3D11_BIND_VERTEX_BUFFER,
  };
  winrt::com_ptr<ID3D11Buffer> vertex_buffer;
  D3D11_SUBRESOURCE_DATA sr_data = {
    .pSysMem = data,
  };
  auto hr =
    device->CreateBuffer(&vertex_buff_desc, &sr_data, vertex_buffer.put());
  if (FAILED(hr)) {
    return {};
  }
  return vertex_buffer;
}

}
}
