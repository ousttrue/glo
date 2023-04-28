#include <Windows.h>
#include <grapho/dx11/device.h>
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

static const char* vertex_shader_text = R"(#version 400
uniform mat4 MVP;
in vec2 vPos;
in vec2 vUv;
out vec2 uv;
void main()
{
    gl_Position = MVP * vec4(vPos, 0.0, 1.0);
    uv = vUv;
};
)";

static const char* fragment_shader_text = R"(#version 400
in vec2 uv;
out vec4 FragColor;
uniform sampler2D colorTexture;

void main()
{
    FragColor = texture(colorTexture, uv);
};
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

  while (processMessage()) {
    swapchain->Present(1, 0);
  }

  return 0;
}
