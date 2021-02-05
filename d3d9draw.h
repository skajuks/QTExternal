/*
#ifndef __D3D9Render__
#define __D3D9Render__

#ifdef _WIN32
#pragma once
#endif

#include "esp.h"
#include "Color.h"

namespace D3D9
{
    class D3D9Render
    {
    public:
        D3D9Render( void );
        ~D3D9Render( void );

        void	String( int x, int y, Color color, ID3DXFont* pFont, const char *fmt, ... );
        void	StringOutlined( int x, int y, Color color, ID3DXFont* pFont, const char *fmt, ... );
        void	Rect( int x, int y, int l, int h, Color color );
        void	BorderBox( int x, int y, int l, int h, int thickness, Color color );
        void	BorderBoxOutlined( int x, int y, int l, int h, int thickness, Color color, Color outlined );
        void	RectOutlined( int x, int y, int l, int h, Color rectcolor, Color outlinedcolor, int thickness );
        void	Line( int x, int y, int x2, int y2, Color color, float thickness = 1 );
        void	GardientRect( int x, int y, int w, int h, int thickness, bool outlined, Color from, Color to, Color Outlined );
    };

    extern D3D9Render*	pRender;
    extern bool				Render( void );
    extern ID3DXFont*		GetFont( std::string szName );
    extern int				getWidth( void );
    extern int				getHeight( void );
    extern HRESULT			CreateOverlay( std::string szGameWindowTitle, std::string szOverlayTitle );
};

#endif*/
