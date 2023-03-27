#pragma once
#include <cuber/mesh.h>
#include <d3dcompiler.h>
#include <dxgi.h>
#include <string_view>
#include <winrt/base.h>

namespace cuber::dx11 {

winrt::com_ptr<ID3DBlob> CompileShader(std::string_view src, const char *entry,
                                       const char *target);
DXGI_FORMAT DxgiFormat(const cuber::VertexLayout &layout);

} // namespace cuber