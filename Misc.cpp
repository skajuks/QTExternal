#include "Misc.h"
#include "Functions.h"
#include "offsets.hpp"
#include "widget.h"
#include "csmath.h"
#include <iostream>
#include <QMessageBox>

using namespace hazedumper::netvars;
using namespace hazedumper::signatures;

bool bhop_enabled = 0;
bool blockbot_enabled = 0;

float distance_factor = 2.f;
float trajectory_factor = 0.45f;

//uintptr_t cl_sidespeed = Memory.FindSignature((char*)"client.dll", "0xF3\0x0F\0x10\0x05\0x00\0x00\0x00\0x00\0xF3\0x0F\0x11\0x44\0x24\0x00\0x81\0x74\0x24\0x00\0x00\0x00\0x00\0x00\0xD9\0x44\0x24\0x14\0xEB\0x07", "xxxx????xxxxx?xxx?????xxxxxx") + 0x4;
//uintptr_t cl_forwardspeed = Memory.FindSignature((char*)"client.dll", "0xF3\0x0F\0x10\0x05\0x00\0x00\0x00\0x00\0xF3\0x0F\0x11\0x44\0x24\0x00\0x81\0x74\0x24\0x00\0x00\0x00\0x00\0x00\0xEB\0x37", "xxxx????xxxxx?xxx?????xx") + 0x4;

bool _Bhop::bhopEnabled(){
    if (bhop_enabled)
        return true;
    else
        return false;
}

void _Bhop::setBhopEnabled(bool state){
    bhop_enabled = state;
}

void Misc::changeFovN(const ClientInfo& localPlayer, int fov){
    Memory.writeMem<int>(localPlayer.entity + m_iDefaultFOV, fov);
}

void Misc::changeFov(int fov){
    uintptr_t localPlayer = Functions::getLocalPlayer();
    Memory.writeMem<int>(localPlayer + m_iDefaultFOV, fov);
}

void _Bhop::initialize()
{
    while(true)
    {
        uintptr_t localPlayer = Functions::getLocalPlayer();
        if(bhopEnabled())
        {

            if(GetAsyncKeyState(VK_SPACE))
            {
                uintptr_t onGround = Functions::getFlag(localPlayer);
                if(!Math::checkForVelocity(localPlayer) && onGround & (1 << 0))
                    Memory.writeMem<int>(gameModule + dwForceJump, 6);
            }

        }

        /*if(blockbot_enabled){
            if(Functions::isAlive(localPlayer))
                continue;

            uintptr_t target = NULL;
            bool blocked = false;

            if(GetKeyState(0x45) & 1){
                if(target == NULL){
                    target =
                }

            }
        }*/

        Sleep(1);
    }
}







