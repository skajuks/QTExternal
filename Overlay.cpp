//overlay.cpp
// ExternalOverlay.cpp : Defines the entry point for the application.
//
#include <Dwmapi.h>
#include "paint.h"
#include "esp.h"
#include <thread>

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR overlayWindowName[100] = L"Overlay";  // main window class name & The title bar text
LPCSTR targetWindowName = "Counter-Strike: Global Offensive";  // main window class name & The title bar text
HWND targetHWND, overlayHWND;
int width, height;
Paint paint;

// Forward declarations of functions included in this code module:
ATOM                registerClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

int ESP::run()
{
    //UNREFERENCED_PARAMETER(hPrevInstance);
    //UNREFERENCED_PARAMETER(lpCmdLine);

    registerClass(NULL);

    targetHWND = FindWindowA(0, targetWindowName);
    if (targetHWND) {
        RECT rect;
        GetWindowRect(targetHWND, &rect);
        width = rect.right - rect.left;
        height = rect.bottom - rect.top;
    }
    else
        return FALSE;

    // Perform application initialization:
    if (!InitInstance(NULL, SW_SHOW)){
        return FALSE;
    }
    paint = Paint(overlayHWND,targetHWND,width,height);
    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0)){
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        TranslateMessage(&msg);
        DispatchMessage(&msg);

        RECT rect;
        GetWindowRect(targetHWND, &rect);
        width = rect.right - rect.left;
        height = rect.bottom - rect.top;

        MoveWindow(overlayHWND, rect.left, rect.top, width, height, true);

    }

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM registerClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = 0;
    wcex.hCursor        = LoadCursor(nullptr, IDC_CROSS);
    wcex.hbrBackground  = NULL;//CreateSolidBrush(RGB(0, 0, 0));
    wcex.lpszMenuName   = overlayWindowName;
    wcex.lpszClassName  = overlayWindowName;
    wcex.hIconSm        = 0;

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   overlayHWND = CreateWindowExW(WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED, overlayWindowName, overlayWindowName, WS_POPUP,
      1, 1, width, height, nullptr, nullptr, hInstance, nullptr);

   if (!overlayHWND){
      return FALSE;
   }
   SetLayeredWindowAttributes(overlayHWND, RGB(0, 0, 0), 0, LWA_COLORKEY);

   ShowWindow(overlayHWND, nCmdShow);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam){
    switch (message){
    case WM_PAINT:
        paint.render();
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

