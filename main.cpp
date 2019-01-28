#ifndef UNICODE
#define UNICODE
#endif 

#include <windows.h>
#include <stdio.h>
#include <math.h>
#include "types.cpp"


void ParseObjFile(const char* file_name);
void ParseTgaImage(const char* file_name, Texture* texture);

Color BLUE = {255,0,0,0};
Color GREEN = {0,255,0,0};
Color RED = {0,0,255,0};
Color WHITE = {255,255,255,0};

Texture TEXTURE_DIFFUSE;
Texture TEXTURE_NM;
Texture TEXTURE_SPEC;

#include "draw.cpp"
#include "parsers.cpp"

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void PaintWindow(HWND hwnd) ;

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR pCmdLine, int nCmdShow)
{
#if 0
  AllocConsole();
  freopen("CONIN$", "r",stdin);
  freopen("CONOUT$", "w",stdout);
  freopen("CONOUT$", "w",stderr);
#endif
  // Register the window class.
  const wchar_t CLASS_NAME[]  = L"Tony's 3D Renderer";

  WNDCLASS wc = { };

  wc.lpfnWndProc   = WindowProc;
  wc.hInstance     = hInstance;
  wc.lpszClassName = CLASS_NAME;

  RegisterClass(&wc);
  //const char* file = "african_head";
  const char* file = "diablo3_pose";
  char buffer[1024];
  strcpy(buffer, file);
  strcat(buffer, ".obj");
  ParseObjFile(buffer);
  strcpy(buffer, file);
  strcat(buffer, "_diffuse.tga");
  ParseTgaImage(buffer, &TEXTURE_DIFFUSE);
  strcpy(buffer, file);
  strcat(buffer, "_nm.tga");
  ParseTgaImage(buffer, &TEXTURE_NM);
  strcpy(buffer, file);
  strcat(buffer, "_spec.tga");
  ParseTgaImage(buffer, &TEXTURE_SPEC);
  // Create the window.

  HWND hwnd = CreateWindowEx(
      0,                              // Optional window styles.
      CLASS_NAME,                     // Window class
      L"Tony's awesome renderer",    // Window text
      WS_OVERLAPPEDWINDOW,            // Window style

      // Size and position
      //CW_USEDEFAULT, CW_USEDEFAULT,
      1500, 700,
      //CW_USEDEFAULT, CW_USEDEFAULT,
      800, 800,

      NULL,       // Parent window    
      NULL,       // Menu
      hInstance,  // Instance handle
      NULL        // Additional application data
      );

  if (hwnd == NULL)
  {
    return 0;
  }

  ShowWindow(hwnd, nCmdShow);

  // Run the message loop.

  MSG msg = { };
  while (GetMessage(&msg, NULL, 0, 0))
  {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
    PaintWindow(hwnd);
  }
  FreeConsole();

  free(TEXTURE_DIFFUSE.buffer);
  return 0;
}

void PaintWindow(HWND hwnd) {
        //PAINTSTRUCT paint;
        //BeginPaint(hwnd, &paint);
        HDC hdc = GetDC(hwnd);
        RECT rect;
        GetClientRect(hwnd, &rect);
        Bitmap bitmap;
        bitmap.width = rect.right - rect.left;
        bitmap.height = rect.bottom - rect.top;

        BITMAPINFO bitmap_info = {};
        bitmap_info.bmiHeader.biSize = sizeof(bitmap_info);
        bitmap_info.bmiHeader.biWidth =   bitmap.width;
        bitmap_info.bmiHeader.biHeight = bitmap.height;
        bitmap_info.bmiHeader.biPlanes = 1;
        bitmap_info.bmiHeader.biBitCount = 32;
        bitmap_info.bmiHeader.biCompression = BI_RGB;

        bitmap.buffer = (DWORD*) malloc(sizeof(DWORD) * bitmap.width * bitmap.height);
        bitmap.z_buffer = (float*) malloc(sizeof(DWORD) * (DWORD)bitmap.width * (DWORD)bitmap.height);
        bitmap.shadow_buffer = (float*) malloc(sizeof(DWORD) * (DWORD)bitmap.width * (DWORD)bitmap.height);
        for(int i = 0; i < bitmap.width * bitmap.height; i++) {
          bitmap.z_buffer[i] = -9999999.0;
          bitmap.shadow_buffer[i] = -9999999.0;
        }
        Draw(bitmap);
        //EndPaint(hwnd, &paint);
        int success = StretchDIBits(hdc,
            0, 0, bitmap.width, bitmap.height,
            0, 0, bitmap.width, bitmap.height,
            bitmap.buffer, &bitmap_info,
            DIB_RGB_COLORS, SRCCOPY);
        if (!success) {
          int x = 1;
        }
        free(bitmap.buffer);
        free(bitmap.z_buffer);
        free(bitmap.shadow_buffer);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch (uMsg)
  {
    case WM_DESTROY:
      PostQuitMessage(0);
      return 0;

    case WM_SIZE:
    {

    }
    return 0;
    case WM_PAINT:
      {
      //PaintWindow(hwnd);
      }
      return 0;

  }
  return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
