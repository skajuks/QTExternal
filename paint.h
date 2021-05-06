#pragma once

#include <string> //save error
#include <Windows.h>
#include "Visuals.h"

#include <d3d9.h>
#include <d3dx9.h>
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")

#include <DxErr.h> //get error from error code
#pragma comment(lib, "dxerr.lib")
#pragma comment(lib, "legacy_stdio_definitions.lib")

struct Color{
    int a, r, g, b;
};

class Paint{
private:
    static IDirect3D9Ex* d3dObject; //= NULL; //used to create device
    static IDirect3DDevice9Ex* d3dDevice; //= NULL; //contains functions like begin and end scene
    static D3DPRESENT_PARAMETERS d3dparams; //parameters for creating device
    static ID3DXFont* d3dFont; //= 0; // font used when displaying text
    static HWND targetWnd;
    static int d3D9Init(HWND hWnd);

public:
    Paint();
    Paint(HWND hWnd, HWND targetWnd, int width, int height);
    int render();
    static int width;
    static int height;

    std::string watermark = "QTExternal v1.2 by chady";

    static void renderEnabledFeatures();
    static void toggleEspMaster(bool state);
    static void String(char* String, int x, int y, int a, int r, int g, int b);
    static void StringOutlined(char* String, int x, int y, int a, int r, int g, int b, int l_a, int l_r, int l_g, int l_b);
    static void Rect( int x, int y, int l, int h, int a, int r, int g, int b);
    static void BorderBox( int x, int y, int l, int h, int thickness, int a, int r, int g, int b);
    static void BorderBoxOutlined( int x, int y, int l, int h, int thickness, int a, int r, int g, int b, int a_o, int r_o, int g_o, int b_o);
    static void BorderBoxOutlined( int x, int y, int l, int h, int thickness, Color color);
    static void GardientRect( int x, int y, int w, int h, int thickness, bool outlined, int a_from, int r_from, int g_from, int b_from, int a_to, int r_to, int g_to, int b_to, int a_outline, int r_outline, int g_outline, int b_outline);
    static void Line( int x, int y, int x2, int y2, Color snaplineColor, float thickness);
    static void drawCrosshair();
};
