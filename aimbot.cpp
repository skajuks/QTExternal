#include "aimbot.h"
#include "Functions.h"
#include "offsets.hpp"
#include "float.h"
#include <math.h>
#include "cmath"
#include <iostream>
#include <limits>
#define _USE_MATH_DEFINES

#undef max

/*
        to do list:

        make a check for weapon in hand and ajust recoil accordingly
        make a check if weapon is scoped
        remove recoil for snipers
        shoot only if target can be killed ( no dm protection)
        add closest entity by fov
        add triggerbot
        move shit like vector and calc functions into math.cpp and vector.cpp
*/


using namespace hazedumper::signatures;
using namespace hazedumper::netvars;

bool aimbot_toggle = false;
int aimKey = 0x01;
int bone_int = 8;
int aimfov = 20;
float smooth = 1.f;
float recoil_perc = 1.f;
bool enable_silentaim = false;

char* pistols[] ={0,0,0,0,0};   // add for every gun

struct VECTOR2{
    float x,y;
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

    float x;
    float y;
    float z;
};

class CInput {
private:
    BYTE __pad0x01[0xF1]; //we dont need these vars
public:
    DWORD Commands;
    DWORD VerifiedCommands;
};

class CUserCmd {
public:
    DWORD Vft;
    DWORD CmdNumber;
    DWORD TickCount;
    VECTOR3 ViewAngles;
    VECTOR3 AimDirection;
    FLOAT Forwardmove;
    FLOAT Sidemove;
    FLOAT Upmove;
    DWORD Buttons;
    BYTE Impulse;
private:
    BYTE __pad0x01[0x03];
public:
    DWORD WeaponSelect;
    DWORD WeaponSubtype;
    DWORD RandomSeed;
    WORD MouseDx;
    WORD MouseDy;
    BOOLEAN HasBeenPredicted;
private:
    BYTE __pad0x02[0x1B];
};
class CVerifiedUserCmd {
public:
    CUserCmd Command;
    DWORD CRC;
};



VECTOR2 CalcDistance(float currx, float curry, float newx, float newy) {
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

void normalizeAngles(float* viewAnglex, float* viewAngley) {
    if (*viewAnglex > 89)
        *viewAnglex -= 360;
    if (*viewAnglex < -89)
        *viewAnglex += 360;
    if (*viewAngley > 180)
        *viewAngley -= 360;
    if (*viewAngley < -180)
        *viewAngley += 360;
}

VECTOR3 CalcAngle(VECTOR3 src, VECTOR3 dst)
{
    VECTOR3 angles;
    VECTOR3 delta;
    delta.x = src.x - dst.x;
    delta.y = src.y - dst.y;
    delta.z = src.z - dst.z;

    float hyp = sqrt(delta.x * delta.x + delta.y * delta.y + delta.z * delta.z);

    angles.x = atan(delta.z / hyp) * 180.0 / M_PI;
    angles.y = atan(delta.y / delta.x) * 180.0 / M_PI;
    angles.z = 0.0f;

    if (delta.x >= 0.0f)
        angles.y += 180.0f;

    return angles;
}

VECTOR3 PlayerPos(uintptr_t entity){
    VECTOR3 pos = Memory.readMem<VECTOR3>(entity + m_vecOrigin);
    return pos;
}

VECTOR3 getBoneMatrix(uintptr_t entity, int bone) {
    uintptr_t bones = Memory.readMem<uintptr_t>(entity + m_dwBoneMatrix);
    VECTOR3 boneMatrix;
    boneMatrix.x = Memory.readMem<float>(bones + 0x30 * bone + 0xC);
    boneMatrix.y = Memory.readMem<float>(bones + 0x30 * bone + 0x1C);
    boneMatrix.z = Memory.readMem<float>(bones + 0x30 * bone + 0x2C);
    return boneMatrix;
}

VECTOR3 PlayerAngles(){
    VECTOR3 angl = Memory.readMem<VECTOR3>(engineModulep + dwClientState_ViewAngles);
    return angl;
}

void ClampAngles(float* anglex, float* angley)
{
    if (*anglex > 89.0f) *anglex = 89.0f;
    else if (*anglex < -89.0f) *anglex = -89.0f;

    if (*angley > 180.0f) *angley = 180.0f;
    else if (*angley < -180.0f) *angley = -180.0f;

}

VECTOR3 Smooth(float smooth, VECTOR3 currentAngle, VECTOR3 aimAngle)
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


// ui fucntions

void Aim::toggleAimbotOnKey(bool state){
    aimbot_toggle = state;
}

void Aim::setFOVonSlider(int value){
    aimfov = value;
}

void Aim::setSmoothOnSlider(float value){
    smooth = value;
}

void Aim::setRecoilControlPerc(float value){
    recoil_perc = value;
}

void Aim::toggleSilentAim(bool state){
    enable_silentaim = state;
}


void doSilentAim(VECTOR2 angles, bool CanShoot){
    CInput Input;
    DWORD clientState;

    Input = Memory.readMem<CInput>(gameModule + dwInput);
    clientState = Functions::getClientState();

    Memory.writeMem<BYTE>(engineModule + dwbSendPackets, 0); //disable packet send

    int desiredCMD;
    desiredCMD = Memory.readMem<int>(clientState + clientstate_last_outgoing_command);
    desiredCMD += 2; // +2 for incomming one

    DWORD incommingUserCMD = Input.Commands + (desiredCMD % 150) * sizeof(CUserCmd);
    DWORD currentUserCMD = Input.Commands + ((desiredCMD - 1) % 150) * sizeof(CUserCmd);
    DWORD verifiedCurUserCMD = Input.VerifiedCommands + ((desiredCMD - 1) % 150) * sizeof(CVerifiedUserCmd);

    int CMDNumber = NULL;
    while(CMDNumber < desiredCMD)
        CMDNumber = Memory.readMem<int>(incommingUserCMD + 0x04);

    CUserCmd cmd; // read the usercmd
    cmd = Memory.readMem<CUserCmd>(currentUserCMD);

    cmd.ViewAngles.x = angles.x;
    cmd.ViewAngles.y = angles.y;

    if(CanShoot) // triggers shooting if set to tru
        cmd.Buttons |= true;

    Memory.writeMem<CUserCmd>(currentUserCMD, cmd);
    Memory.writeMem<CUserCmd>(verifiedCurUserCMD, cmd);

    Memory.writeMem<BYTE>(engineModule + dwbSendPackets, 1); //restore packet sending :)
}


VECTOR3 pPos;
VECTOR3 entBone;
VECTOR3 pAngle;
VECTOR2 normalAngles;
VECTOR3 targetBones;
VECTOR3 old_recoil{0.f, 0.f, 0.f};

VECTOR2 getClosestEnemyByAngle(){               //redo this function, doesnt need while loop and toggle check

    uintptr_t localPlayer = Functions::getLocalPlayer();
    float oldDistancex = std::numeric_limits<float>::max();;
    float oldDistancey = std::numeric_limits<float>::max();;
    VECTOR3 angleTo;
    VECTOR3 AimAngle;
    VECTOR2 readyAngles;
    uintptr_t target = NULL;
    if(GetAsyncKeyState(aimKey)){
        VECTOR3 recoil = Memory.readMem<VECTOR3>(localPlayer + m_aimPunchAngle);
        recoil *= (recoil_perc / 90.f) * 2.f;


        for(short int i = 0; i < 32; i++){
            uintptr_t entity = Functions::getEntity(i);
            if(Functions::getTeam(entity) != Functions::getTeam(localPlayer) && Functions::isAlive(entity)){
                pPos = PlayerPos(localPlayer);
                entBone = getBoneMatrix(entity, bone_int);
                pAngle = PlayerAngles();
                pAngle.z = Memory.readMem<float>(localPlayer + m_vecViewOffset + 0x8);
                pPos.z += pAngle.z;
                angleTo = CalcAngle(pPos, entBone);
                auto newDistance = CalcDistance(pAngle.x, pAngle.y, angleTo.x, angleTo.y);
                if(newDistance.x < oldDistancex && newDistance.y < oldDistancey && newDistance.x <= aimfov && newDistance.y <= aimfov){
                    oldDistancex = newDistance.x;
                    oldDistancey = newDistance.y;
                    targetBones = entBone;
                    target = entity;
                }
            }
        }
        if(target != NULL){
            std::cout << angleTo.x << angleTo.y << angleTo.z << std::endl;
            AimAngle = CalcAngle(pPos, targetBones);
            AimAngle = Smooth(smooth, pAngle, AimAngle - recoil);
            normalizeAngles(&AimAngle.x , &AimAngle.y);
            readyAngles = {AimAngle.x, AimAngle.y};
            if(enable_silentaim)
                doSilentAim(readyAngles, true);
            else
                Memory.writeMem<VECTOR2>(engineModulep + dwClientState_ViewAngles, readyAngles);

        }
    }
}

void Aim::AIMBOT(){
    while(true){
        if(aimbot_toggle){
            //if(GetKeyState(VK_HOME) & 1)
            getClosestEnemyByAngle();

        }
        Sleep(1);
    }
}


















