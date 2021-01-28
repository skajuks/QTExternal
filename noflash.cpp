#include "noflash.h"
#include "Functions.h"
#include "offsets.hpp"

using namespace hazedumper::netvars;
using namespace hazedumper::signatures;

bool toggleNoFlash = false;
int flashAmmount = 0;

DWORD m_hTonemapController = 0x31CC; //DT_CSPlayer
DWORD fm_bUseCustomAutoExposureMax = 0x9D4;
DWORD fm_bUseCustomAutoExposureMin = 0x9D8; //DT_EnvTonemapController
DWORD fm_flCustomAutoExposureMin = 0x9DC; //DT_EnvTonemapController
DWORD fm_flCustomAutoExposureMax = 0x9E0; //DT_EnvTonemapController



void Flash::setEnabledNoFlash(bool state){
    toggleNoFlash = state;
}

void Flash::setNightmodeAmount(float amount){
    int tmcIndex = Memory.readMem<int>(Functions::getLocalPlayer() + m_hTonemapController) & 0xFFF;
    DWORD tmcHandle = Functions::getEntity(tmcIndex);
    Memory.writeMem<int>(tmcHandle + fm_bUseCustomAutoExposureMin, 1);
    Memory.writeMem<int>(tmcHandle + fm_bUseCustomAutoExposureMax, 1);
    Memory.writeMem<float>(tmcHandle + fm_flCustomAutoExposureMin, amount);
    Memory.writeMem<float>(tmcHandle + fm_flCustomAutoExposureMax, amount);
}


void Flash::noflash(){
    setNightmodeAmount(0.09f);
    while(true){
        if(toggleNoFlash){
            uintptr_t localPlayer = Functions::getLocalPlayer();
            int flashdur = Functions::getFlashDuration(localPlayer);
            if(flashdur > 0)
                Functions::setFlashDuration(localPlayer, flashAmmount);
            Sleep(1);
        }
    }
}
