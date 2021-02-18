#include "csmath.h"
#include "offsets.hpp"
#include "Functions.h"
#include <Windows.h>
#include "float.h"
#include <csmath.h>
#define _USE_MATH_DEFINES
#include <iostream>
#include <limits>
#include "Structs.h" // lai header fwd decl dabu defu Entity

using namespace hazedumper::netvars;
using namespace hazedumper::signatures;


bool Math::checkForVelocity(const Entity& local_entity)
{
    VECTOR3 vel = local_entity.vecVelocity;
    int sum = vel.x + vel.y + vel.z;
    if(sum != 0)
        return false;
    return true;
}

VECTOR3 Math::WorldToScreen(const VECTOR3 pos, view_matrix_t* matrix, int width, int height) {

    // this shit should be moved to math cpp

    VECTOR3 out;
    float _x = matrix->matrix[0] * pos.x + matrix->matrix[1] * pos.y + matrix->matrix[2] * pos.z + matrix->matrix[3];
    float _y = matrix->matrix[4] * pos.x + matrix->matrix[5] * pos.y + matrix->matrix[6] * pos.z + matrix->matrix[7];
    out.z = matrix->matrix[12] * pos.x + matrix->matrix[13] * pos.y + matrix->matrix[14] * pos.z + matrix->matrix[15];

    _x *= 1.f / out.z;
    _y *= 1.f / out.z;

    out.x = width * .5f;
    out.y = height * .5f;

    out.x += 0.5f * _x * width + 0.5f;
    out.y -= 0.5f * _y * height + 0.5f;

    return out;
}

VECTOR2 Math::CalcDistance(float currx, float curry, float newx, float newy) {
    VECTOR2 distance;

    distance.x = newx - currx;
    if (distance.x < -89)
        distance.x += 360;
    else if (distance.x > 89)
        distance.x -= 360;
    if (distance.x < 0.0)
        distance.x = -distance.x;

    distance.y = newy - curry;
    if (distance.y < -180)
        distance.y += 360;
    else if (distance.y > 180)
        distance.y -= 360;
    if (distance.y < 0.0)
        distance.y = -distance.y;

    return distance;
}

void Math::normalizeAngles(float* viewAnglex, float* viewAngley) {
    if (*viewAnglex > 89)
        *viewAnglex -= 360;
    if (*viewAnglex < -89)
        *viewAnglex += 360;
    if (*viewAngley > 180)
        *viewAngley -= 360;
    if (*viewAngley < -180)
        *viewAngley += 360;
}

VECTOR3 Math::CalcAngle(VECTOR3 src, VECTOR3 dst)
{
    VECTOR3 angles;
    VECTOR3 delta;
    delta.x = src.x - dst.x;
    delta.y = src.y - dst.y;
    delta.z = src.z - dst.z;

    double hyp = sqrt(delta.x * delta.x + delta.y * delta.y);

    angles.x = (float)(atan(delta.z / hyp) * 180 / M_PI);
    angles.y = (float)(atan(delta.y / delta.x) * 180 / M_PI);
    angles.z = 0.0f;

    if (delta.x >= 0.0f)
        angles.y += 180.0f;

    return angles;
}

VECTOR3 Math::PlayerPos(uintptr_t entity){
    VECTOR3 pos = Memory.readMem<VECTOR3>(entity + m_vecOrigin);
    return pos;
}

VECTOR3 Math::getBoneMatrix(uintptr_t entity, int bone) {
    uintptr_t bones = Memory.readMem<uintptr_t>(entity + m_dwBoneMatrix);
    VECTOR3 boneMatrix;
    boneMatrix.x = Memory.readMem<float>(bones + 0x30 * bone + 0xC);
    boneMatrix.y = Memory.readMem<float>(bones + 0x30 * bone + 0x1C);
    boneMatrix.z = Memory.readMem<float>(bones + 0x30 * bone + 0x2C);
    return boneMatrix;
}

VECTOR3 Math::PlayerAngles(){
    VECTOR3 angl = Memory.readMem<VECTOR3>(engineModulep + dwClientState_ViewAngles);
    return angl;
}

void Math::ClampAngles(float* anglex, float* angley)
{
    if (*anglex > 89.0f) *anglex = 89.0f;
    else if (*anglex < -89.0f) *anglex = -89.0f;

    if (*angley > 180.0f) *angley = 180.0f;
    else if (*angley < -180.0f) *angley = -180.0f;

}

VECTOR3 Math::Smooth(float smooth, VECTOR3 currentAngle, VECTOR3 aimAngle)
{
    VECTOR3 angle;
    VECTOR3 dest = angle;
    if (smooth > 0)
    {
        VECTOR3 delta;
        delta.y = currentAngle.y - aimAngle.y;
        delta.x = currentAngle.x - aimAngle.x;
        normalizeAngles(&delta.x, &delta.y);
        ClampAngles(&delta.x, &delta.y);
        dest.y = currentAngle.y - delta.y / (smooth);
        dest.x = currentAngle.x - delta.x / (smooth);
        normalizeAngles(&dest.x, &dest.y);
        ClampAngles(&dest.x, &dest.y);
    }

    angle = dest;

    return angle;

}
