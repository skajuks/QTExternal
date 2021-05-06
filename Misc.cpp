#include "Misc.h"
#include "Functions.h"
#include "offsets.hpp"
#include "widget.h"
#include "csmath.h"
#include <iostream>
#include <QMessageBox>
#include <thread>

using namespace hazedumper::netvars;
using namespace hazedumper::signatures;

struct customAutoExposure{
    DWORD useExposureMax = 1;
    DWORD useExposureMin = 1;
    DWORD customExposureMax;
    DWORD customExposureMin;
};

customAutoExposure* cAX;

bool bhop_enabled = false;
bool dmexploit_enabled = false;
bool perfect_nade = false;

bool toggleNoFlash = false;
int flashAmmount = 0;

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

void Misc::setDMExploitEnabled(bool state){
    dmexploit_enabled = !dmexploit_enabled;
}

void Misc::setPerfectNadeEnabled(){
    perfect_nade = !perfect_nade;
}

void Misc::miscItemsThreaded(const Entity& localPlayer, const ClientInfo& ci){
    while(1 < 2){
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        if(dmexploit_enabled || perfect_nade){

            if(localPlayer.health <= 0)
                continue;

            if(dmexploit_enabled){  // execute if player is alive
                Functions::clientCmd_Unrestricted("open_buymenu");
                Sleep(500);
            }

            if(perfect_nade){
                if(LocalPlayer::getSpeed(localPlayer) > 3)
                    continue;

                if(Math::PlayerAngles().x != -89)
                    continue;

                if(LocalPlayer::getActiveWeapon(ci) != 44)  // he nade
                    continue;

                if((GetAsyncKeyState(0x02) & 0x8000) != 0){
                    std::this_thread::sleep_for(std::chrono::milliseconds(800));
                    if(!((GetAsyncKeyState(0x02) & 0x8000) != 0))
                        continue;
                    Functions::clientCmd_Unrestricted("+attack");
                    std::this_thread::sleep_for(std::chrono::milliseconds(80));
                    Functions::clientCmd_Unrestricted("-attack2");
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                    Functions::clientCmd_Unrestricted("-attack");
                }
            }
        }


    }
}

void Misc::doBlockBot(float side, float forward){
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
    Memory.readMemTo<CUserCmd>(currentUserCMD, &cmd);

    cmd.Sidemove = side;
    cmd.Forwardmove = forward;

    Memory.writeMemFrom<CUserCmd>(currentUserCMD, &cmd);
    Memory.writeMemFrom<CUserCmd>(verifiedCurUserCMD, &cmd);

    Memory.writeMem<BYTE>(engineModule + dwbSendPackets, 1); //restore packet sending :)

}

void Misc::doorSpammer(int use){
    Memory.writeMem<int>(Memory.readMem<int>(gameModule + pOffsets.dwUse), use);
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
