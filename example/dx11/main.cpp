#include <Windows.h>

const auto CLASS_NAME = "dx11class";
const auto WINDOW_NAME = "dx11window";
const auto WIDTH = 640;
const auto HEIGHT = 480;

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
  }

  return 0;
}
