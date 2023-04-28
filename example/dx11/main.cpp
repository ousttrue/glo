#include <Windows.h>
#include <grapho/dx11/device.h>
#include <grapho/dx11/drawable.h>
#include <grapho/dx11/shader.h>
#include <iostream>
#include <stdint.h>

const auto CLASS_NAME = "dx11class";
const auto WINDOW_NAME = "dx11window";
const auto WIDTH = 640;
const auto HEIGHT = 480;

struct rgba
{
  uint8_t r;
  uint8_t g;
  uint8_t b;
  uint8_t a;
};

struct float2
{
  float x, y;
};
struct float3
{
  float x, y, z;
};
struct Vertex
{
  float2 positon;
  float2 uv;
};
auto s = 0.5f;
/// CCW
/// 3   2
/// +---+
/// |   |
/// +---+
/// 0   1
static const struct Vertex vertices[] = {
  { { -s, -s }, { 0.f, 1.f } },
  { { s, -s }, { 1.f, 1.f } },
  { { s, s }, { 1.f, 0.f } },
  { { -s, s }, { 0.f, 0.f } },
};
static const uint32_t indices[] = {
  0, 1, 2, //
  2, 3, 0, //
};
static rgba pixels[4] = {
  { 255, 0, 0, 255 },
  { 0, 255, 0, 255 },
  { 0, 0, 255, 255 },
  { 255, 255, 255, 255 },
};

static const char* shader_text = R"(
#pragma pack_matrix(row_major)
struct vs_in {
    float2 pos: POSITION;
    float2 uv: TEXCOORD;
};
struct vs_out {
    float4 position_clip: SV_POSITION;
    float2 uv: TEXCOORD;
};

vs_out vs_main(vs_in IN) {
  vs_out OUT = (vs_out)0; // zero the memory first
  OUT.position_clip = float4(IN.pos.xy, 0, 1);
  OUT.uv = IN.uv;
  return OUT;
}

Texture2D colorTexture;
SamplerState colorSampler;

float4 ps_main(vs_out IN) : SV_TARGET {
  float4 texel = colorTexture.Sample(colorSampler, IN.uv);
  return texel;
}
)";

static LRESULT CALLBACK
WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch (message) {
    case WM_CREATE:
      return 0;

    case WM_DESTROY: {
      PostQuitMessage(0);
      return 0;
    }

    case WM_ERASEBKGND:
      return 0;

    case WM_SIZE:
      // _state.width = LOWORD(lParam);
      // _state.height = HIWORD(lParam);
      // _is_maximized = wParam == SIZE_MAXIMIZED;
      return 0;

    case WM_PAINT: {
      PAINTSTRUCT ps;
      HDC hdc = BeginPaint(hWnd, &ps);
      EndPaint(hWnd, &ps);
      return 0;
    }
  }

  return DefWindowProcA(hWnd, message, wParam, lParam);
}

int WINAPI
WinMain(HINSTANCE hInstance,
        HINSTANCE hPrevInstance,
        LPSTR lpCmdLine,
        int nCmdShow)
{
  WNDCLASSEXA windowClass = {
    .cbSize = (UINT)sizeof(WNDCLASSEXW),
    .style = CS_HREDRAW | CS_VREDRAW,
    .lpfnWndProc = WndProc,
    .hInstance = hInstance,
    .hCursor = LoadCursor(NULL, IDC_ARROW),
    .lpszClassName = CLASS_NAME,
  };
  if (!RegisterClassExA(&windowClass)) {
    return 1;
  }

  auto hWnd = CreateWindowA(CLASS_NAME,
                            WINDOW_NAME,
                            WS_OVERLAPPEDWINDOW,
                            CW_USEDEFAULT,
                            CW_USEDEFAULT,
                            WIDTH,
                            HEIGHT,
                            nullptr, // We have no parent window.
                            nullptr, // We aren't using menus.
                            hInstance,
                            nullptr);
  if (!hWnd) {
    return 2;
  }
  ShowWindow(hWnd, SW_SHOW);

  auto device = grapho::dx11::CreateDevice();
  if (!device) {
    return 3;
  }

  auto swapchain = grapho::dx11::CreateSwapchain(device, hWnd);
  if (!swapchain) {
    return 4;
  }

  auto vs_compiled =
    grapho::dx11::CompileShader(shader_text, "vs_main", "vs_5_0");
  if (!vs_compiled) {
    std::cout << vs_compiled.error() << std::endl;
    return 5;
  }
  winrt::com_ptr<ID3D11VertexShader> vs;
  auto hr = device->CreateVertexShader((*vs_compiled)->GetBufferPointer(),
                                       (*vs_compiled)->GetBufferSize(),
                                       NULL,
                                       vs.put());
  assert(SUCCEEDED(hr));

  auto ps_compiled =
    grapho::dx11::CompileShader(shader_text, "ps_main", "ps_5_0");
  if (!ps_compiled) {
    std::cout << ps_compiled.error() << std::endl;
    return 6;
  }
  winrt::com_ptr<ID3D11PixelShader> ps;
  hr = device->CreatePixelShader((*ps_compiled)->GetBufferPointer(),
                                 (*ps_compiled)->GetBufferSize(),
                                 NULL,
                                 ps.put());
  assert(SUCCEEDED(hr));

  winrt::com_ptr<ID3D11Buffer> vertex_buffer;
  {
    D3D11_BUFFER_DESC vertex_buff_desc = {
      .ByteWidth = static_cast<uint32_t>(sizeof(vertices)),
      .Usage = D3D11_USAGE_DEFAULT,
      .BindFlags = D3D11_BIND_VERTEX_BUFFER,
    };
    D3D11_SUBRESOURCE_DATA sr_data = {
      .pSysMem = vertices,
    };
    hr = device->CreateBuffer(&vertex_buff_desc, &sr_data, vertex_buffer.put());
    assert(SUCCEEDED(hr));
  }

  winrt::com_ptr<ID3D11Buffer> index_buffer;
  {
    D3D11_BUFFER_DESC index_buff_desc = {
      .ByteWidth = static_cast<uint32_t>(sizeof(indices)),
      .Usage = D3D11_USAGE_DEFAULT,
      .BindFlags = D3D11_BIND_INDEX_BUFFER,
    };
    D3D11_SUBRESOURCE_DATA sr_data = {
      .pSysMem = indices,
    };
    hr = device->CreateBuffer(&index_buff_desc, &sr_data, index_buffer.put());
    assert(SUCCEEDED(hr));
  }

  grapho::VertexLayout layouts[] = {
    {
      .id = { "POSITION", 0, 0 },
      .type = grapho::ValueType::Float,
      .count = 2,
      .offset = offsetof(Vertex, positon),
      .stride = sizeof(Vertex),
    },
    {
      .id = { "TEXCOORD", 0, 0 },
      .type = grapho::ValueType::Float,
      .count = 2,
      .offset = offsetof(Vertex, uv),
      .stride = sizeof(Vertex),
    },
  };
  grapho::dx11::VertexSlot slots[] = {
    {
      .vertex_buffer = vertex_buffer,
      .stride = sizeof(Vertex),
    },
  };
  auto drawable = grapho::dx11::Drawable::Create(
    device, *vs_compiled, layouts, slots, index_buffer);

  winrt::com_ptr<ID3D11RasterizerState> rs;
  {
    D3D11_RASTERIZER_DESC rs_desc = {
      .FillMode = D3D11_FILL_SOLID,
      .CullMode = D3D11_CULL_BACK,
      .FrontCounterClockwise = true,
      .ScissorEnable = false,
      .MultisampleEnable = false,
    };
    hr = device->CreateRasterizerState(&rs_desc, rs.put());
    if (FAILED(hr)) {
      return 7;
    }
  }

  winrt::com_ptr<ID3D11Texture2D> texture;
  winrt::com_ptr<ID3D11ShaderResourceView> srv;
  winrt::com_ptr<ID3D11SamplerState> sampler;
  {
    D3D11_TEXTURE2D_DESC desc = {
      .Width = 2,
      .Height = 2,
      .MipLevels = 1,
      .ArraySize = 1,
      .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
      .SampleDesc{ .Count = 1 },
      .BindFlags = D3D11_BIND_SHADER_RESOURCE,
    };
    D3D11_SUBRESOURCE_DATA initData{
      .pSysMem = pixels,
      .SysMemPitch = 8,
      .SysMemSlicePitch = 16,
    };
    hr = device->CreateTexture2D(&desc, &initData, texture.put());
    if (FAILED(hr)) {
      return 8;
    }
    D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc = {
      .Format = desc.Format,
      .ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D,
      .Texture2D{
        .MostDetailedMip = 0,
        .MipLevels = desc.MipLevels,
      },
    };
    hr = device->CreateShaderResourceView(texture.get(), &viewDesc, srv.put());
    if (FAILED(hr)) {
      return 9;
    }
    D3D11_SAMPLER_DESC sampler_desc = {
      .Filter = D3D11_FILTER_MIN_MAG_MIP_POINT,
      .AddressU = D3D11_TEXTURE_ADDRESS_WRAP,
      .AddressV = D3D11_TEXTURE_ADDRESS_WRAP,
      .AddressW = D3D11_TEXTURE_ADDRESS_WRAP,
    };
    auto hr = device->CreateSamplerState(&sampler_desc, sampler.put());
    if (FAILED(hr)) {
      return 10;
    }
  }

  auto processMessage = []() {
    MSG msg = {};
    while (true) {
      if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
        if (msg.message == WM_QUIT) {
          return false;
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
      } else {
        break;
      }
    }
    return true;
  };

  winrt::com_ptr<ID3D11Texture2D> pBackBuffer;
  swapchain->GetBuffer(0, IID_PPV_ARGS(pBackBuffer.put()));

  winrt::com_ptr<ID3D11DeviceContext> context;
  device->GetImmediateContext(context.put());
  float clear_color[4]{ 0.2f, 0.2f, 0.2f, 0 };
  while (processMessage()) {
    winrt::com_ptr<ID3D11RenderTargetView> rtv;
    hr = device->CreateRenderTargetView(pBackBuffer.get(), NULL, rtv.put());
    assert(SUCCEEDED(hr));
    context->ClearRenderTargetView(rtv.get(), clear_color);

    // pipeline
    ID3D11RenderTargetView* rtv_list[] = {
      rtv.get(),
    };
    context->OMSetRenderTargets(1, rtv_list, nullptr);
    D3D11_VIEWPORT viewport = { 0.0f, 0.0f, WIDTH, HEIGHT, 0.0f, 1.0f };
    context->RSSetViewports(1, &viewport);

    {
      context->VSSetShader(vs.get(), NULL, 0);
      context->PSSetShader(ps.get(), NULL, 0);

      context->RSSetState(rs.get());
      ID3D11ShaderResourceView* srvs[] = {
        srv.get(),
      };
      context->PSSetShaderResources(0, std::size(srvs), srvs);
      ID3D11SamplerState* samplers[] = {
        sampler.get(),
      };
      context->PSSetSamplers(0, std::size(samplers), samplers);
      drawable->Draw(context, std::size(indices));
    }

    swapchain->Present(1, 0);
  }

  return 0;
}
