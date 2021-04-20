#ifndef CSMATH_H
#define CSMATH_H
#include <windows.h>

#pragma once

#define RAD2DEG(rad)	( static_cast<float>(rad) * static_cast<float>(180.f / M_PI) )
#define DEG2RAD(deg)	( static_cast<float>(deg) * static_cast<float>(M_PI / 180.f) )

struct VECTOR2{
    float x,y;
};

struct view_matrix_t {
    float matrix[16];
};

class player_info {
private:
    char __pad[0x10];
public:
    char name[32];
};

class VECTOR3{
public:
    VECTOR3(void)
    {
        Init();
    }
    VECTOR3(float x, float y, float z)
    {
        Init(x, y, z);
    }
    VECTOR3(const float* clr)
    {
        Init(clr[0], clr[1], clr[2]);
    }

    void Init(float ix = 0.0f, float iy = 0.0f, float iz = 0.0f)
    {
        x = ix;
        y = iy;
        z = iz;
    }

    float operator[](int i) const
    {
        return ((float*)this)[i];
    }
    float& operator[](int i)
    {
        return ((float*)this)[i];
    }

    VECTOR3& operator*(float fl)
    {
        x *= fl;
        y *= fl;
        z *= fl;
        return *this;
    }

    VECTOR3& operator*=(float fl)
    {
        x *= fl;
        y *= fl;
        z *= fl;
        return *this;
    }

    VECTOR3 operator-(const VECTOR3& v) const
    {
        return VECTOR3(x - v.x, y - v.y, z - v.z);
    }

    VECTOR3 operator+(const VECTOR3& v) const
    {
        return VECTOR3(x + v.x, y + v.y, z + v.z);
    }
    float x;
    float y;
    float z;
};

// forward declare Entity
struct Entity;

class Math
{
public:
    static float vectorNormalize(VECTOR2& v);
    static void AngleVectors(const VECTOR3& angles, VECTOR2* forward, VECTOR2* right, VECTOR2* up);
    static bool checkForVelocity(const Entity& local_entity);
    static VECTOR3 WorldToScreen(const VECTOR3 pos, view_matrix_t* matrix, int width, int height);
    static VECTOR2 CalcDistance(float currx, float curry, float newx, float newy);
    static void normalizeAngles(float* viewAnglex, float* viewAngley);
    static VECTOR3 CalcAngle(VECTOR3 src, VECTOR3 dst);
    static VECTOR3 PlayerPos(uintptr_t entity);
    static VECTOR3 getBoneMatrix(uintptr_t entity, int bone);
    static VECTOR3 PlayerAngles();
    static void ClampAngles(float* anglex, float* angley);
    static VECTOR3 Smooth(float smooth, VECTOR3 currentAngle, VECTOR3 aimAngle);
    static bool equalVector(VECTOR3 source, VECTOR3 compare);
};

#endif // MATH_H
