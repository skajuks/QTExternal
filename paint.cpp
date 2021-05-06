#include "paint.h"
#include "Functions.h"
#include "offsets.hpp"
#include "csmath.h"
#include "entity.h"
#include <string>
#include <iostream>

#define MAX_RENDER_HEIGHT 50
#define MIN_RENDER_WIDTH 100

using namespace hazedumper::netvars;
using namespace hazedumper::signatures;

IDirect3D9Ex* Paint::d3dObject = NULL;
IDirect3DDevice9Ex* Paint::d3dDevice = NULL;
ID3DXFont* Paint::d3dFont = 0;
D3DPRESENT_PARAMETERS Paint::d3dparams; //parameters for creating device
HWND Paint::targetWnd;
int Paint::width;
int Paint::height;
bool esp_master = false;

extern ClientInfo ci[64];  // ci[0] = localplayer
extern Entity e[64];   // ci[0] = localplayer

extern toggleStateData stateData[6];

int Paint::d3D9Init(HWND hWnd){

    if (FAILED(Direct3DCreate9Ex(D3D_SDK_VERSION, &d3dObject))){
        exit(1);
    }

    ZeroMemory(&d3dparams, sizeof(d3dparams));

    d3dparams.BackBufferWidth = width;
    d3dparams.BackBufferHeight = height;
    d3dparams.Windowed = TRUE;
    d3dparams.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dparams.hDeviceWindow = hWnd;
    d3dparams.MultiSampleQuality = D3DMULTISAMPLE_NONE;
    d3dparams.BackBufferFormat = D3DFMT_A8R8G8B8;
    d3dparams.EnableAutoDepthStencil = TRUE;
    d3dparams.AutoDepthStencilFormat = D3DFMT_D16;

    HRESULT res = d3dObject->CreateDeviceEx(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dparams, 0, &d3dDevice);

    if (FAILED(res)){
        std::wstring ws(DXGetErrorString(res));
        std::string str(ws.begin(), ws.end());
        std::wstring ws2(DXGetErrorDescription(res));
        std::string str2(ws2.begin(), ws2.end());
        std::string error = "Error: " + str + " error description: " + str2;
        exit(1);
    }

    D3DXCreateFont(d3dDevice, 14, 0, FW_NORMAL, 1, false, DEFAULT_CHARSET, OUT_DEVICE_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Arial", &d3dFont);

    return 0;

}

Paint::Paint() {};

Paint::Paint(HWND hWnd, HWND targetWnd, int width, int height){
    this->width = width;
    this->height = height;
    this->targetWnd = targetWnd;
    d3D9Init(hWnd);
}

void Paint::drawCrosshair(){
    //Line(width / 2, height / 2 - 10, width / 2, height / 2 + 10, crosshairColor, 2);
    //Line(width / 2 - 10, height / 2, width / 2 + 10, height / 2, crosshairColor, 2);
    Rect(width / 2 - 3, height / 2 - 3, 6, 6, 0,0,0,0);
}

void Paint::toggleEspMaster(bool state){
    esp_master = state;
}

void Paint::renderEnabledFeatures(){
    int height_offset = MAX_RENDER_HEIGHT;
    for(int i = 0; i < 6; i++){
        StringOutlined((char*)stateData[i].text, MIN_RENDER_WIDTH, height_offset, 255,0,1,0,255,255,255,255);
        StringOutlined(stateData[i].state ? (char*)"ON" : (char*)"OFF", MIN_RENDER_WIDTH + 100, height_offset, 255,0,1,0,255,stateData[i].state ? 0 : 255 ,stateData[i].state ? 255 : 0 ,0);
        height_offset += 20;
    }
}


int Paint::render()
{
    if (d3dDevice == nullptr)
        return 1;
    d3dDevice->Clear(0, 0, D3DCLEAR_TARGET, 0, 1.0f, 0);
    d3dDevice->BeginScene();

    if (targetWnd == GetForegroundWindow()){
        if(esp_master){
            int entityIndex = 1;
            do {
               if(entityIndex >= 64) break;      // checks only player entities [max = 64]

               if(e[entityIndex].health > 0 && e[entityIndex].team != e[0].team){
                   try {
                       Glow::ProcessD3D9Render(ci[entityIndex], e[entityIndex], entityIndex);
                   }  catch(...) {
                       continue;
                   }

               }
            } while(ci[entityIndex++].nextEntity);
        }

        StringOutlined((char*)watermark.c_str(),5,30,255,0,1,0, 255, 255, 255 ,255);   // watermark
        renderEnabledFeatures();
        Sleep(1);
    }


    d3dDevice->EndScene();
    d3dDevice->PresentEx(0, 0, 0, 0, 0);
    Sleep(1);

    return 0;
}

// paint functions

void Paint::String(char* String, int x, int y, int a, int r, int g, int b)
{
    RECT FontPos;
    FontPos.left = x;
    FontPos.top = y;
    d3dFont->DrawTextA(0, String, strlen(String), &FontPos, DT_NOCLIP, D3DCOLOR_ARGB(a, r, g, b));
}

void Paint::StringOutlined(char* String, int x, int y, int a, int r, int g, int b, int l_a, int l_r, int l_g, int l_b)
{
    Paint::String(String, x - 1, y, a, r, g, b);
    Paint::String(String, x, y - 1, a, r, g, b);
    Paint::String(String, x + 1, y, a, r, g, b);
    Paint::String(String, x, y + 1, a, r, g, b);
    Paint::String(String, x, y, l_a, l_r, l_g, l_b);
}

void Paint::Rect( int x, int y, int l, int h, int a, int r, int g, int b)
{
    D3DRECT rect = { x, y, x + l, y + h };
    d3dDevice->Clear( 1, &rect, D3DCLEAR_TARGET, D3DCOLOR_ARGB( a,r,g,b ), 0, 0 );
}

void Paint::BorderBox( int x, int y, int l, int h, int thickness, int a, int r, int g, int b)
{
    Paint::Rect( x, y, l, thickness, a,r,g,b );
    Paint::Rect( x, y, thickness, h, a,r,g,b );
    Paint::Rect( x + l, y, thickness, h, a,r,g,b );
    Paint::Rect( x, y + h, l+thickness, thickness, a,r,g,b );
}

void Paint::BorderBoxOutlined( int x, int y, int l, int h, int thickness, int a, int r, int g, int b, int a_o, int r_o, int g_o, int b_o)
{
    Paint::BorderBox( x, y, l, h, thickness, a,r,g,b );
    Paint::BorderBox( x - 1, y - 1, l + 2, h + 2, 1, a_o, r_o, g_o, b_o);
    Paint::BorderBox( x + 1, y + 1, l - 2, h - 2, 1, a_o, r_o, g_o, b_o);
}

void Paint::BorderBoxOutlined( int x, int y, int l, int h, int thickness, Color color)
{
    Paint::BorderBox( x, y, l, h, thickness, color.a, color.r, color.g, color.b);
    Paint::BorderBox( x - 1, y - 1, l + 2, h + 2, 1, 255,0,1,0);
    Paint::BorderBox( x + 1, y + 1, l - 2, h - 2, 1, 255,0,1,0);
}

void Paint::GardientRect( int x, int y, int w, int h, int thickness, bool outlined, int a_from, int r_from, int g_from, int b_from, int a_to, int r_to, int g_to, int b_to, int a_outline, int r_outline, int g_outline, int b_outline)
{
    float a = ( ( float )a_to - ( float )a_from ) / h;
    float r = ( ( float )r_to - ( float )r_from ) / h;
    float g = ( ( float )g_to - ( float )g_from ) / h;
    float b = ( ( float )b_to - ( float )b_from ) / h;

    for( int i = 0; i < h; i++ )
    {
        int A = a_from + a * i;
        int R = r_from + r * i;
        int G = g_from + g * i;
        int B = b_from + b * i;
        Paint::Rect( x, y + i, w, 1, a, r, g ,b );
    }
    if( outlined )
    {
        Paint::BorderBox( x - thickness, y - thickness, w + thickness, h + thickness, thickness, a_outline, r_outline, g_outline, b_outline);
    }
}
void Paint::Line( int x, int y, int x2, int y2, Color snaplineColor, float thickness )
{
    ID3DXLine* LineL;
    D3DXCreateLine(d3dDevice, &LineL);
    D3DXVECTOR2 points[ 2 ];
    points[ 0 ].x = x;
    points[ 1 ].x = x2;
    points[ 0 ].y = y;
    points[ 1 ].y = y2;
    LineL->SetWidth( thickness );
    LineL->Draw(points,  2, D3DCOLOR_ARGB(snaplineColor.a, snaplineColor.r, snaplineColor.g, snaplineColor.b));
    LineL->Release();
}
