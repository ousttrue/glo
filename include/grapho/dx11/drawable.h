#pragma once
#include "../vertexlayout.h"
#include <assert.h>
#include <d3d11.h>
#include <span>
#include <winrt/base.h>

namespace grapho {
namespace dx11 {

inline DXGI_FORMAT
DxgiFormat(const grapho::VertexLayout& layout)
{
  switch (layout.type) {
    case grapho::ValueType::Float:
      switch (layout.count) {
        case 2:
          return DXGI_FORMAT_R32G32_FLOAT;
        case 3:
          return DXGI_FORMAT_R32G32B32_FLOAT;
        case 4:
          return DXGI_FORMAT_R32G32B32A32_FLOAT;
      }
  }
  throw std::invalid_argument("not implemented");
}

struct VertexSlot
{
  winrt::com_ptr<ID3D11Buffer> vertex_buffer;
  uint32_t stride;
};

struct Drawable
{
  winrt::com_ptr<ID3D11InputLayout> InputLayout;
  std::vector<VertexSlot> Slots;
  winrt::com_ptr<ID3D11Buffer> IndexBuffer;

  static std::shared_ptr<Drawable> Create(
    const winrt::com_ptr<ID3D11Device>& device,
    const winrt::com_ptr<ID3DBlob>& vs_compiled,
    std::span<const VertexLayout> layouts,
    std::span<const VertexSlot> slots,
    const winrt::com_ptr<ID3D11Buffer>& index_buffer = {})
  {
    std::vector<D3D11_INPUT_ELEMENT_DESC> inputElementDesc;
    for (size_t i = 0; i < layouts.size(); ++i) {
      auto& layout = layouts[i];
      inputElementDesc.push_back({
        .SemanticName = layout.id.semantic_name.c_str(),
        .SemanticIndex = layout.id.semantic_index,
        .Format = DxgiFormat(layout),
        .InputSlot = layout.id.slot,
        .AlignedByteOffset = layout.offset,
        .InputSlotClass = layout.divisor ? D3D11_INPUT_PER_INSTANCE_DATA
                                         : D3D11_INPUT_PER_VERTEX_DATA,
        .InstanceDataStepRate = layout.divisor,
      });
    }
    auto ptr = std::make_shared<Drawable>();
    ptr->IndexBuffer = index_buffer;
    auto hr = device->CreateInputLayout(inputElementDesc.data(),
                                        inputElementDesc.size(),
                                        vs_compiled->GetBufferPointer(),
                                        vs_compiled->GetBufferSize(),
                                        ptr->InputLayout.put());
    if (FAILED(hr)) {
      return nullptr;
    }
    for (auto& slot : slots) {
      ptr->Slots.push_back(slot);
    }
    return ptr;
  }

  void Draw(const winrt::com_ptr<ID3D11DeviceContext>& context,
            uint32_t indexCount)
  {
    context->IASetInputLayout(InputLayout.get());
    std::vector<ID3D11Buffer*> buffers;
    std::vector<uint32_t> strides;
    std::vector<uint32_t> offsets;
    for (auto slot : Slots) {
      buffers.push_back(slot.vertex_buffer.get());
      strides.push_back(slot.stride);
      offsets.push_back(0);
    }
    context->IASetVertexBuffers(
      0, Slots.size(), buffers.data(), strides.data(), offsets.data());
    context->IASetIndexBuffer(IndexBuffer.get(), DXGI_FORMAT_R32_UINT, 0);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context->DrawIndexed(indexCount, 0, 0);
  }
};

}
}