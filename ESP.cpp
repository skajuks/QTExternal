#include "esp.h"
#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <fcntl.h>
#include <dwmapi.h>


//d3d9 includes
#include <d3d9.h>
#include <d3dx9.h>

//load d3d9 libs

#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")

#define WS_EX_LAYERED 0x00080000
#define LWA_ALPHA 0x00000002
#define ARGB_TRANS 0x00000000

LPDIRECT3D9EX d3d;
LPDIRECT3DDEVICE9EX d3ddev;

int w_width = 1920;
int w_height = 1080;

ID3DXFont* font = NULL;
HRESULT hr = D3D_OK;
MARGINS windowMargins = { -1, -1, -1, -1};
const char* name = "overlay";

HWND CSGO_hWnd = NULL;
DWORD CSGO_PID = 0;
HANDLE CSGO_hProc = NULL;

bool init_ok = false;
COORD w_pos = {50,50};
COORD w_res = {1920, 1080};

LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

void DrawString(LPCSTR string, int x, int y, COLORREF color, ID3DXFont* font){

    RECT font_rect;
    SetRect(&font_rect, x, y, w_res.X, w_res.Y);
    font->DrawText(NULL, (const wchar_t*)string, -1, &font_rect, DT_LEFT | DT_NOCLIP | DT_SINGLELINE, color);

}

HRESULT D3DStartup(HWND hWnd){

    BOOL bCompOk = false;
    D3DPRESENT_PARAMETERS pp;
    DWORD msqAAQuality = 0;
    HRESULT hr;

    DwmIsCompositionEnabled(&bCompOk);
    if (!bCompOk) return E_FAIL;

    hr = Direct3DCreate9Ex(D3D_SDK_VERSION, &d3d);
    if (FAILED(hr)){

        MessageBoxA(0, "Failed to create d3d obj", "error", 0);
        return E_FAIL;
    }

    ZeroMemory(&pp, sizeof(pp));
    pp.Windowed = TRUE;
    pp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    pp.BackBufferFormat = D3DFMT_A8R8G8B8;

    if(SUCCEEDED(d3d->CheckDeviceMultiSampleType(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, D3DFMT_A8B8G8R8, TRUE, D3DMULTISAMPLE_NONMASKABLE, &msqAAQuality))){

        pp.MultiSampleType = D3DMULTISAMPLE_NONMASKABLE;
        pp.MultiSampleQuality = msqAAQuality - 1;
    }
    else
    {
        pp.MultiSampleType = D3DMULTISAMPLE_NONE;
    }

    hr = d3d->CreateDeviceEx(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &pp, NULL, &d3ddev);

    if(FAILED(hr)){

        MessageBoxA(0, "Failed to create d3d dev object", "error", 0);
        return E_FAIL;

    }

}

VOID Render(VOID){

    COLORREF color = 0xFFF00FF;

    if(d3ddev == NULL) return;

    d3ddev->Clear(0, NULL, D3DCLEAR_TARGET, ARGB_TRANS, 1.0f, 0);

    d3ddev->BeginScene();

    // this is where the cheat should be added
    DrawString("This is a test string", 5, 20, color, font);

    d3ddev->EndScene();

    d3ddev->PresentEx(NULL,NULL,NULL,NULL,NULL);

}

void D3DShutdown(void){

    if(d3ddev != NULL)
        d3ddev->Release();
    if(d3d != NULL)
        d3d->Release();
}

int run(HINSTANCE hInstance, HINSTANCE, LPSTR, INT){

    HWND hWnd= NULL;
    MSG uMsg;
    HRESULT hr;
    long Loops = 0;
    WNDCLASSEX wc = {sizeof(WNDCLASSEX), NULL, WindowProc, NULL, NULL, hInstance, LoadIcon(NULL, IDI_APPLICATION),
                    LoadCursor(NULL, IDC_ARROW), NULL, NULL, (LPCWSTR)name, LoadIcon(NULL, IDI_APPLICATION)};
    RegisterClassEx(&wc);

    hWnd = CreateWindowEx(WS_EX_TOPMOST | WS_EX_COMPOSITED | WS_EX_TRANSPARENT | WS_EX_LAYERED, (LPCWSTR)name, (LPCWSTR)name, WS_POPUP,
                          w_pos.X, w_pos.Y, w_res.X, w_res.Y, NULL, NULL, hInstance, NULL);

    hr = DwmExtendFrameIntoClientArea(hWnd, &windowMargins);

    if(SUCCEEDED(D3DStartup(hWnd))){

        D3DXCreateFont(d3ddev, 20, 10, FW_NORMAL, 0, FALSE, DEFAULT_CHARSET,
                       OUT_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE, TEXT("Arial"), &font);

        ShowWindow(hWnd, SW_SHOWDEFAULT);
        UpdateWindow(hWnd);

        while(true){

            if(PeekMessage(&uMsg, NULL, 0, 0, PM_REMOVE)){

                if (uMsg.message == WM_QUIT)
                    break;

                TranslateMessage(&uMsg);
                DispatchMessage(&uMsg);
            }

            if (CSGO_hWnd == NULL)
                CSGO_hWnd = FindWindow(NULL, L"Counter-Strike: Global Offensive");
            if ((CSGO_hWnd != NULL) && (CSGO_PID == 0))
                GetWindowThreadProcessId(CSGO_hWnd, &CSGO_PID);

            if ((CSGO_hWnd == NULL) && (CSGO_PID != 0))
                CSGO_hProc = OpenProcess(PROCESS_VM_READ, false, CSGO_PID);

            if (CSGO_hProc != NULL){
                if ((!init_ok) || (Loops % 20) == 0){
                    RECT client_rect;
                    GetClientRect(CSGO_hWnd, &client_rect);
                    w_res.X = client_rect.right;
                    w_res.Y = client_rect.bottom;

                    RECT bounding_rect;
                    GetWindowRect(CSGO_hWnd, &bounding_rect);
                    if(!init_ok){
                        if((w_pos.X != bounding_rect.left) || (w_pos.Y != bounding_rect.top)){
                            MoveWindow(hWnd, bounding_rect.left, bounding_rect.top, client_rect.right, client_rect.bottom, false);
                            w_pos.X = bounding_rect.left;
                            w_pos.Y = bounding_rect.top;
                        }
                    }
                    else
                    {
                        if((bounding_rect.left == 0) && (bounding_rect.top == 0)){
                            MoveWindow(hWnd, bounding_rect.left - 1, bounding_rect.top - 1, client_rect.right, client_rect.bottom, false);
                        }
                        MoveWindow(hWnd, bounding_rect.left, bounding_rect.top, client_rect.right, client_rect.bottom, false);

                    }
                    init_ok = true;
                }
            }

            if(Loops % 10 == 0){
                if(FindWindow(NULL, L"Counter-Strike: Global Offensive") == NULL){
                    SendMessage(hWnd, WM_CLOSE, NULL, NULL);
                }
            }
            Loops++;
            if(Loops > 100) Loops = 0;

            Render();
        }
    }
    D3DShutdown();
    return 0;

}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam){

    switch(message){
    case WM_DESTROY:
    {
        PostQuitMessage(0);
        return 0;
    } break;
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}
