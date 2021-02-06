#include "paint.h"
#include "Functions.h"
#include "offsets.hpp"

using namespace hazedumper::netvars;
using namespace hazedumper::signatures;

class player_info {
private:
    char __pad[0x10];
public:
    char name[32];
};

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

struct Vector3 {
    float x, y, z;
};

struct view_matrix_t {
    float matrix[16];
};

struct Vector3 WorldToScreen(const struct Vector3 pos, struct view_matrix_t matrix) {
    struct Vector3 out;
    float _x = matrix.matrix[0] * pos.x + matrix.matrix[1] * pos.y + matrix.matrix[2] * pos.z + matrix.matrix[3];
    float _y = matrix.matrix[4] * pos.x + matrix.matrix[5] * pos.y + matrix.matrix[6] * pos.z + matrix.matrix[7];
    out.z = matrix.matrix[12] * pos.x + matrix.matrix[13] * pos.y + matrix.matrix[14] * pos.z + matrix.matrix[15];

    _x *= 1.f / out.z;
    _y *= 1.f / out.z;

    int width = 1600;
    int height = 900;

    out.x = width * .5f;
    out.y = height * .5f;

    out.x += 0.5f * _x * width + 0.5f;
    out.y -= 0.5f * _y * height + 0.5f;

    return out;
}
int Paint::render()
{
    if (d3dDevice == nullptr)
        return 1;
    d3dDevice->Clear(0, 0, D3DCLEAR_TARGET, 0, 1.0f, 0);
    d3dDevice->BeginScene();

    if (targetWnd == GetForegroundWindow())
    {
        uintptr_t clientState = Functions::getClientState();
        uintptr_t uinfoTable = Memory.readMem<uintptr_t>(clientState + dwClientState_PlayerInfo);
        uintptr_t items = Memory.readMem<std::uintptr_t>(Memory.readMem<uintptr_t>(uinfoTable + 0x40) + 0xC);
        view_matrix_t vm = Memory.readMem<view_matrix_t>(gameModule + dwViewMatrix);
        uintptr_t localPlayer = Functions::getLocalPlayer();
        int myTeam = Functions::getTeam(localPlayer);
        for (short int i = 1; i < 32; i++) {
            uintptr_t entity = Memory.readMem<uintptr_t>(gameModule + dwEntityList + i * 0x10);
            if (entity != 0 && Functions::isAlive(entity) && Functions::getTeam(entity) != myTeam) {
                player_info player = Memory.readMem<player_info>(Memory.readMem<uintptr_t>((items + 0x28) + (i * 0x34)));
                Vector3 pos = Memory.readMem<Vector3>(entity + m_vecOrigin);
                Vector3 head = {head.x = pos.x, head.y = pos.y, head.z = pos.z + 75.f};
                Vector3 screenpos = WorldToScreen(pos, vm);
                Vector3 screenhead = WorldToScreen(head, vm);
                int height = screenpos.y - screenhead.y;
                int width = height / 2;
                screenhead.y += 30;

                if(screenpos.z >= 0.01f){
                    BorderBoxOutlined(screenhead.x - width / 2,screenhead.y,width,height,2,100,100,100,255, 255, 0 ,1, 0);
                    StringOutlined((char*)player.name,screenhead.x - width / 2,screenhead.y - 10,255,0,1,0);
                }
            }

        }
        //check if esp is toggled on
        //  entity loop
        //  find entity name and weapon
        // w2screen matrix
        // if entity is correct, set entity box and name/weaopn esp

        StringOutlined((char*)"QTExternal v1.2 by chady",5,30,255,0,1,0);   // watermark
        //StringOutlined((char*)"HE grenade",140,360,255,0,1,0);

    }

    d3dDevice->EndScene();
    d3dDevice->PresentEx(0, 0, 0, 0, 0);
    Sleep(1);

    return 0;
}

void Paint::String(char* String, int x, int y, int a, int r, int g, int b)
{
    RECT FontPos;
    FontPos.left = x;
    FontPos.top = y;
    d3dFont->DrawTextA(0, String, strlen(String), &FontPos, DT_NOCLIP, D3DCOLOR_ARGB(a, r, g, b));
}

void Paint::StringOutlined(char* String, int x, int y, int a, int r, int g, int b)
{
    this->String(String, x - 1, y, a, r, g, b);
    this->String(String, x, y - 1, a, r, g, b);
    this->String(String, x + 1, y, a, r, g, b);
    this->String(String, x, y + 1, a, r, g, b);
    this->String(String, x, y, 255, 255, 255, 255);
}

void Paint::Rect( int x, int y, int l, int h, int a, int r, int g, int b)
{
    D3DRECT rect = { x, y, x + l, y + h };
    d3dDevice->Clear( 1, &rect, D3DCLEAR_TARGET, D3DCOLOR_ARGB( a,r,g,b ), 0, 0 );
}

void Paint::BorderBox( int x, int y, int l, int h, int thickness, int a, int r, int g, int b)
{
    this->Rect( x, y, l, thickness, a,r,g,b );
    this->Rect( x, y, thickness, h, a,r,g,b );
    this->Rect( x + l, y, thickness, h, a,r,g,b );
    this->Rect( x, y + h, l+thickness, thickness, a,r,g,b );
}

void Paint::BorderBoxOutlined( int x, int y, int l, int h, int thickness, int a, int r, int g, int b, int a_o, int r_o, int g_o, int b_o)
{
    this->BorderBox( x, y, l, h, thickness, a,r,g,b );
    this->BorderBox( x - 1, y - 1, l + 2, h + 2, 1, a_o, r_o, g_o, b_o);
    this->BorderBox( x + 1, y + 1, l - 2, h - 2, 1, a_o, r_o, g_o, b_o);
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
        this->Rect( x, y + i, w, 1, a, r, g ,b );
    }
    if( outlined )
    {
        this->BorderBox( x - thickness, y - thickness, w + thickness, h + thickness, thickness, a_outline, r_outline, g_outline, b_outline);
    }
}
