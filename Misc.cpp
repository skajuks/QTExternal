#include "Misc.h"
#include "Functions.h"
#include "offsets.hpp"
#include "widget.h"
#include <iostream>
#include <QMessageBox>

using namespace hazedumper::netvars;
using namespace hazedumper::signatures;

struct Vector3{
    float x,y,z;
};

bool bhop_enabled = 0;

bool _Bhop::bhopEnabled(){
    if (bhop_enabled)
        return true;
    else
        return false;
}

void _Bhop::setBhopEnabled(bool state){
    bhop_enabled = state;
}

void Misc::changeFov(int fov){
    uintptr_t localPlayer = Functions::getLocalPlayer();
    Memory.writeMem<int>(localPlayer + m_iDefaultFOV, fov);
}


bool checkForVelocity(uintptr_t localPlayer)
{
    Vector3 vel = Memory.readMem<Vector3>(localPlayer + m_vecVelocity);
    int sum = vel.x + vel.y + vel.z;
    if(sum != 0)
        return false;
    else
        return true;
}

void _Bhop::initialize()
{
    while(true)
    {

        if(bhopEnabled())
        {

            if(GetAsyncKeyState(VK_SPACE))
            {
                static Vector3 prevViewAngles;
                Vector3 currentViewAngles = Memory.readMem<Vector3>(engineModulep + dwClientState_ViewAngles);
                uintptr_t localPlayer = Functions::getLocalPlayer();
                uintptr_t onGround = Functions::getFlag(localPlayer);
                if(!checkForVelocity(localPlayer) && onGround & (1 << 0))
                    Memory.writeMem<int>(gameModule + dwForceJump, 6);
                /*if(!checkForVelocity(localPlayer) && onGround != (1 << 0))
                    if(currentViewAngles.y > prevViewAngles.y)
                        Memory.writeMem<int>(gameModule + dwForceLeft, 6);
                    else if (currentViewAngles.y < prevViewAngles.y)
                        Memory.writeMem<int>(gameModule + dwForceRight, 6);
                    prevViewAngles = currentViewAngles;*/
            }

        }
        Sleep(1);
    }
}







