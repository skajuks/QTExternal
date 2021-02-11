#include "paint.h"
#include "Functions.h"
#include "offsets.hpp"
#include "csmath.h"
#include <string>
#include <iostream>

using namespace hazedumper::netvars;
using namespace hazedumper::signatures;

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

// screen res
int taskbar_offset = 40;


// toggles
bool master_esp_toggle = false;
bool boxes_enabled = false;
bool snapl_enabled = false;
bool weapon_health_enabled = false;
bool boxes_by_health_enabled = false;

// init colors
Color snaplineColor = {255,100,100,100};
Color espBoxColor = {100,100,100,255};
Color crosshairColor = {255,50,205,50};

void Paint::StateChanged(int funct){
    switch(funct){
    case 1: master_esp_toggle = !master_esp_toggle;                 // toggle esp master
    case 2: boxes_enabled = !boxes_enabled;                         // toggle esp boxes
    case 3: weapon_health_enabled = !weapon_health_enabled;         // toggle esp health, active weapon and player name
    case 4: snapl_enabled = !snapl_enabled;                         // toggle snaplines
    case 5: boxes_by_health_enabled = !boxes_by_health_enabled;     // toggle esp draw by health
    }
}

void Paint::colorChanged(int funct, int a, int r, int g, int b){
    switch(funct){
    case 1: snaplineColor.a = a; snaplineColor.r = r; snaplineColor.g = g; snaplineColor.b = b;  // change snapline color
    case 2: espBoxColor.a = a; espBoxColor.r = r; espBoxColor.g = g; espBoxColor.b = b;          // change esp box color
    }
}

void Paint::drawCrosshair(){
    Line(width / 2, height / 2 - 10, width / 2, height / 2 + 10, crosshairColor, 2);
    Line(width / 2 - 10, height / 2, width / 2 + 10, height / 2, crosshairColor, 2);
    Rect(width / 2 - 3, height / 2 - 3, 6, 6, 0,0,0,0);
}


int Paint::render()
{
    if (d3dDevice == nullptr)
        return 1;
    d3dDevice->Clear(0, 0, D3DCLEAR_TARGET, 0, 1.0f, 0);
    d3dDevice->BeginScene();

    // =-=-=-=-=-=-=-=-=--=-=-=-=[ Attention ]=-=-=-=-=-=-=-=-=-=-
    //                  BAD CODE ALERT!! YOU HAVE BEED WARNED

    // this whole chunk of code should be integrated with glowesp, so it only loops trough entitylist once
    // same thing could also apply to aimbot entity loop, to save cpu usage
    // should also center out entity name and weapon used
    // esp color to health <-- add
    // redo this thing using structs to minimize rpm/wpm calls
    // create a damn stuct for colors

    if (targetWnd == GetForegroundWindow())
    {
        if(master_esp_toggle){
            uintptr_t clientState = Functions::getClientState();
            uintptr_t uinfoTable = Memory.readMem<uintptr_t>(clientState + dwClientState_PlayerInfo);
            uintptr_t items = Memory.readMem<std::uintptr_t>(Memory.readMem<uintptr_t>(uinfoTable + 0x40) + 0xC);
            view_matrix_t vm = Memory.readMem<view_matrix_t>(gameModule + dwViewMatrix);
            uintptr_t localPlayer = Functions::getLocalPlayer();

            int weaponIdLocal = Memory.readMem<int>(localPlayer + m_hActiveWeapon);
            int weaponEntLocal = Memory.readMem<int>(gameModule + dwEntityList + ((weaponIdLocal & 0xFFF) - 1) * 0x10);
            if(weaponEntLocal != NULL){
                int entityWeaponLocal = Memory.readMem<int>(weaponEntLocal + m_iItemDefinitionIndex);
                if(entityWeaponLocal == 9 || entityWeaponLocal == 11 || entityWeaponLocal == 38 || entityWeaponLocal == 40){
                    drawCrosshair();
                }


                    player_info player = Memory.readMem<player_info>(Memory.readMem<uintptr_t>((items + 0x28) + (i * 0x34)));
                    VECTOR3 pos = Memory.readMem<VECTOR3>(entity + m_vecOrigin);
                    VECTOR3 head = {head.x = pos.x, head.y = pos.y, head.z = pos.z + 75.f};
                    VECTOR3 screenpos = Math::WorldToScreen(pos, vm);
                    VECTOR3 screenhead = Math::WorldToScreen(head, vm);
                    int height_f = screenpos.y - screenhead.y;
                    int width_f = height_f / 2;
                    //screenhead.y += 30;

                    if(screenpos.z >= 0.01f){
                        if(boxes_enabled){
                            if(boxes_by_health_enabled){
                                BorderBoxOutlined(screenhead.x - width_f / 2,screenhead.y,width_f,height_f,2,255,(int)(255 - (entity_hp * 2.55f)),(int)(entity_hp * 2.55f),0, 255, 0 ,1, 0); // 100 100 100 255
                            } else {
                                BorderBoxOutlined(screenhead.x - width_f / 2,screenhead.y,width_f,height_f,2, espBoxColor);
                            }
                        }
                        if(snapl_enabled)
                            Line(width / 2,height - 1,screenhead.x, screenhead.y + height_f, snaplineColor, 1);    // test this

                        if(weapon_health_enabled){
                            int weaponId = Memory.readMem<int>(entity + m_hActiveWeapon);
                            int weaponEnt = Memory.readMem<int>(gameModule + dwEntityList + ((weaponId & 0xFFF) - 1) * 0x10);
                            if(weaponEnt != NULL){
                                int entityWeapon = Memory.readMem<int>(weaponEnt + m_iItemDefinitionIndex);
                                char str[64];
                                snprintf(str, 64, "%dhp", entity_hp);

                                StringOutlined((char*)player.name,screenhead.x - width_f / 2,screenhead.y - 15,255,0,1,0, 255, 255, 255 ,255);
                                StringOutlined((char*)Functions::getActiveWeapon(entityWeapon),screenhead.x - width_f / 2,screenhead.y + height_f + 5,255,0,1,0,255,255,255,255);
                                StringOutlined(str, screenhead.x - width_f / 2,screenhead.y + height_f + 15,255,0,1,0,255,255,255,255);
                            }
                            if(Functions::checkIfScoped(entity))
                                StringOutlined((char*)"Scoped",screenhead.x - width_f / 2,screenhead.y - 30,255,0,1,0, 255, 65 ,255, 0);
                        }
                    }
                }
            }
            StringOutlined((char*)watermark.c_str(),5,30,255,0,1,0, 255, 255, 255 ,255);   // watermark
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
    this->String(String, x - 1, y, a, r, g, b);
    this->String(String, x, y - 1, a, r, g, b);
    this->String(String, x + 1, y, a, r, g, b);
    this->String(String, x, y + 1, a, r, g, b);
    this->String(String, x, y, l_a, l_r, l_g, l_b);
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

void Paint::BorderBoxOutlined( int x, int y, int l, int h, int thickness, Color color)
{
    this->BorderBox( x, y, l, h, thickness, color.a, color.r, color.g, color.b);
    this->BorderBox( x - 1, y - 1, l + 2, h + 2, 1, 255,0,1,0);
    this->BorderBox( x + 1, y + 1, l - 2, h - 2, 1, 255,0,1,0);
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
