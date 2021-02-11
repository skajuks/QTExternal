#include "Misc.h"
#include "Functions.h"
#include "offsets.hpp"
#include "widget.h"
#include "csmath.h"
#include <iostream>
#include <QMessageBox>

using namespace hazedumper::netvars;
using namespace hazedumper::signatures;

struct customAutoExposure{
    DWORD useExposureMax = 1;
    DWORD useExposureMin = 1;
    DWORD customExposureMax;
    DWORD customExposureMin;
};

customAutoExposure* cAX;

bool bhop_enabled = 1;
bool blockbot_enabled = 0;

bool toggleNoFlash = false;
int flashAmmount = 0;

float distance_factor = 2.f;
float trajectory_factor = 0.45f;

//uintptr_t cl_sidespeed = Memory.FindSignature((char*)"client.dll", "0xF3\0x0F\0x10\0x05\0x00\0x00\0x00\0x00\0xF3\0x0F\0x11\0x44\0x24\0x00\0x81\0x74\0x24\0x00\0x00\0x00\0x00\0x00\0xD9\0x44\0x24\0x14\0xEB\0x07", "xxxx????xxxxx?xxx?????xxxxxx") + 0x4;
//uintptr_t cl_forwardspeed = Memory.FindSignature((char*)"client.dll", "0xF3\0x0F\0x10\0x05\0x00\0x00\0x00\0x00\0xF3\0x0F\0x11\0x44\0x24\0x00\0x81\0x74\0x24\0x00\0x00\0x00\0x00\0x00\0xEB\0x37", "xxxx????xxxxx?xxx?????xx") + 0x4;

DWORD m_hTonemapController = 0x31CC; //DT_CSPlayer
DWORD fm_bUseCustomAutoExposureMax = 0x9D4;

bool Misc::bhopEnabled(){
    if (bhop_enabled)
        return true;
    else
        return false;
}

void Misc::setBhopEnabled(bool state){
    bhop_enabled = state;
}

void Misc::changeFov(const ClientInfo& localPlayer, int fov){
    Memory.writeMem<int>(localPlayer.entity + m_iDefaultFOV, fov);
}

void Misc::setBhop(const Entity& local_entity)
{
    if(bhopEnabled())
    {
        if(GetAsyncKeyState(VK_SPACE))
        {
            if(!Math::checkForVelocity(local_entity) && local_entity.flags & (1 << 0))
                Memory.writeMem<int>(gameModule + dwForceJump, 6);
        }
    }
}

void Misc::setNoFlash(const ClientInfo& localPlayer){
    if(toggleNoFlash){
        Functions::setFlashDuration(localPlayer.entity, flashAmmount);
    }
}

void Misc::setEnabledNoFlash(bool state){
    toggleNoFlash = state;
}

void Misc::setNightmodeAmount(const ClientInfo& localPlayer, float amount){
    int tmcIndex = Memory.readMem<int>(localPlayer.entity + m_hTonemapController) & 0xFFF;
    DWORD tmcHandle = Functions::getEntity(tmcIndex);
    cAX->customExposureMax = amount;
    cAX->customExposureMin = amount;
    Memory.writeMemFrom<customAutoExposure*>(tmcHandle, &cAX);

    /*Memory.writeMem<int>(tmcHandle + fm_bUseCustomAutoExposureMin, 1);
    Memory.writeMem<int>(tmcHandle + fm_bUseCustomAutoExposureMax, 1);
    Memory.writeMem<float>(tmcHandle + fm_flCustomAutoExposureMin, amount);
    Memory.writeMem<float>(tmcHandle + fm_flCustomAutoExposureMax, amount);   convert into struct*/
}
