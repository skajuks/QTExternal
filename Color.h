#ifndef __COLOR__
#define __COLOR__

#ifdef _WIN32
#pragma once
#endif

#include "esp.h"

class Color
{
private:
    int	A, R, G, B;

public:
    inline Color( void )
    {

    }
    inline Color( int a, int r, int g, int b )
    {
        A = a;
        R = r;
        G = g;
        B = b;
    }
    inline Color( int r, int g, int b )
    {
        A = 255;
        R = r;
        G = g;
        B = b;
    }
    inline int a( void ) const
    {
        return A;
    }
    inline int r( void ) const
    {
        return R;
    }
    inline int g( void ) const
    {
        return G;
    }
    inline int b( void ) const
    {
        return B;
    }
};

#ifndef WHITE
#define WHITE			Color( 255, 255, 255, 255 )
#endif

#ifndef BLACK
#define BLACK			Color( 255,   0,   0,   0 )
#endif

#ifndef RED
#define RED				Color( 255, 255,   0,   0 )
#endif

#ifndef GREEN
#define GREEN			Color( 255,   0, 255,   0 )
#endif

#ifndef BLUE
#define BLUE			Color( 255,   0,   0, 255 )
#endif

#endif
