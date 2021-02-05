#include "paint.h"


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


int Paint::render()
{
    if (d3dDevice == nullptr)
        return 1;
    d3dDevice->Clear(0, 0, D3DCLEAR_TARGET, 0, 1.0f, 0);
    d3dDevice->BeginScene();

    if (targetWnd == GetForegroundWindow())
    {

        BorderBoxOutlined(100,100,140,250,2,100,100,100,255, 255, 0 ,1, 0);
        StringOutlined((char*)"chady the speedrunner",120,80,255,0,1,0);
        StringOutlined((char*)"HE grenade",140,360,255,0,1,0);

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