#include "Misc.h"
#include "Functions.h"
#include "widget.h"
#include "csmath.h"
#include <iostream>
#include <QMessageBox>
#include <algorithm>
#include <random>
#include <thread>

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
bool autoaccept_enabled = false;
bool radio_commands_enable = false;

bool toggleNoFlash = false;
float flashAmmount = 0.f;

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

void Misc::setAutoAcceptEnabled(){
    autoaccept_enabled = !autoaccept_enabled;
}

bool Misc::returnAutoAcceptState(){
    return autoaccept_enabled;
}

void Misc::setRadioSpamEnabled(){
    radio_commands_enable = !radio_commands_enable;
}

void Functions::loadSkybox(const char* skyname){
    if(!Memory.getAlloc()) return;
    static uintptr_t alloc = reinterpret_cast<uintptr_t>(Memory.getAlloc()) + 434;//143;      // + clientcmd
    static bool injected = false;
    if(!injected){
        BYTE Shellcode[13 + 48] = { 0xB9, 0x00, 0x00, 0x00, 0x00,	// mov	ecx, (skyname address)
                                            0xB8, 0x00, 0x00, 0x00, 0x00,	// mov	eax, (R_LoadNamedSkys address)
                                            0xFF, 0xD0,						// call	eax
                                            0xC3 };
        *reinterpret_cast<uintptr_t*>(&Shellcode[1]) = alloc + 13;
        *reinterpret_cast<uintptr_t*>(&Shellcode[6]) = engineModule + pOffsets.loadNamedSkys;
        strcpy_s(reinterpret_cast<char*>(&Shellcode[13]), 48, skyname);
        Memory.write<BYTE[13 + 48]>(alloc, Shellcode);
        injected = true;
    } else {
        WriteProcessMemory(Memory.handle, reinterpret_cast<LPVOID>(alloc + 13), skyname, strlen(skyname) + 1, 0);
    }
    Memory.createThread(alloc);
}

bool Misc::autoAccept(){
    static bool found = false;
    static bool accepted = false;
    static uintptr_t previous_callback = 0;
    uintptr_t callback;
    Memory.read<uintptr_t>(pOffsets.ConfirmedReservationCallback, callback);

    std::cout << "herere" << std::endl;

    if(callback){

        if(previous_callback != callback){
            previous_callback = callback;
            found = false;
            accepted = false;
        }

        if(!found){
            found = true;
            std::this_thread::sleep_for(std::chrono::seconds(5));
        }
        else if(!accepted){
            accepted = true;
            Memory.createThread(pOffsets.Callback__IsReady);
            printf("Tried to accept the match!");
            return 1;
        }
    } else {
        found = false;
        accepted = false;
    }
    return 0;
}

void Misc::fakeLagThreaded(const Entity& localPlayer, bool toggle){

    bool fakelag_toggle = false;

    while(2 > 1){
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

        if(GetAsyncKeyState(VK_MBUTTON) & 0x8000){
            fakelag_toggle = !fakelag_toggle;
        }

        if(toggle && fakelag_toggle){
            if(localPlayer.health > 0 && !(localPlayer.flags & (1 << 0)) && !(GetAsyncKeyState(VK_LBUTTON) & 0x8000)
                    && !Memory.readMem<bool>(pOffsets.g_bVoiceRecording)){
#ifdef NDEBUG
                //Memory.writeMem<double>(pOffsets.m_flNextCmdTime, FLT_MAX);
#endif
                //Memory.writeMem<BYTE>(engineModule + pyfetch::dwbSendPackets, false);

                std::this_thread::sleep_for(std::chrono::milliseconds(120));    // should be ~8 ticks

#ifdef NDEBUG
                //Memory.writeMem<double>(pOffsets.m_flNextCmdTime, 0.0);
#endif
                //Memory.writeMem<BYTE>(engineModule + hazedumper::signatures::dwbSendPackets, true);


            }

            std::this_thread::sleep_for(std::chrono::milliseconds(35));    // should be ~3 ticks

        }
    }

}

void Misc::miscItemsThreaded(const Entity& localPlayer, const ClientInfo& ci){
    while(1 < 2){
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        if(dmexploit_enabled || perfect_nade || autoaccept_enabled || radio_commands_enable){

            if(!localPlayer.team && autoaccept_enabled){  // check if not in-game
                printf("debug: here");
                if(autoAccept()){
                    autoaccept_enabled = false;
                }
            }

            //if(localPlayer.health >= 0)
               // continue;

            if(dmexploit_enabled || radio_commands_enable){  // execute if player is alive
                if(dmexploit_enabled){
                    Functions::clientCmd_Unrestricted("open_buymenu");
                    Sleep(400);
                }
                if(radio_commands_enable){
                    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
                    std::shuffle(std::begin(radio_commands), std::end(radio_commands), std::default_random_engine(seed));   // shuffle array contents to randomize msg
                    Functions::clientCmd_Unrestricted(radio_commands[0]);
                    Sleep(2000);
                }
            }

            if(perfect_nade){
                if(LocalPlayer::getSpeed(localPlayer) > 3)
                    continue;

                if(Math::PlayerAngles().x != -89)
                    continue;

                if(LocalPlayer::getActiveWeapon(ci) != 44)  // he nade
                    continue;

                if((GetAsyncKeyState(0x02) & 0x8000) != 0){
                    printf("Im here");
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

void Misc::doorSpammer(int use){
    Memory.writeMem<int>(Memory.readMem<int>(gameModule + pOffsets.dwUse), use);
}

void Misc::changeFov(const ClientInfo& localPlayer, int fov){
    Memory.writeMem<int>(localPlayer.entity + pNetVars.m_iDefaultFOV, fov);
}

void Misc::setBhop(const Entity& local_entity)
{
    if(bhopEnabled())
    {
        if(GetAsyncKeyState(VK_SPACE))
        {
            if(!Math::checkForVelocity(local_entity) && local_entity.flags & (1 << 0))
                Memory.writeMem<int>(pOffsets.dwForceJump, 6);
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

void Misc::setNoFlashAmount(float amount){
    flashAmmount = amount;
    std::cout << flashAmmount << std::endl;
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
