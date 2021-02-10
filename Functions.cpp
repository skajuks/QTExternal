#include "Functions.h"
#include "offsets.hpp"
#include "windows.h"
#include <TlHelp32.h>
#include <iostream>

using namespace hazedumper::netvars;
using namespace hazedumper::signatures;

MemMan Memory;

int process_id = Memory.getProcess(L"csgo.exe");
uintptr_t gameModule = Memory.getModule(process_id, L"client.dll");
uintptr_t engineModule = Memory.getModule(process_id, L"engine.dll");
uintptr_t engineModulep = Functions::getClientState();
//Offsets pOffsets = Memory.getOffsets(process_id);



uintptr_t Functions::getClientState() {
    return Memory.readMem<uintptr_t>(engineModule + dwClientState);
}

void Functions::checkForLocalPlayer() {
    uintptr_t localPlayer = Memory.readMem<uintptr_t>(gameModule + dwLocalPlayer);
    if (localPlayer == NULL)
        while (localPlayer == NULL)
            localPlayer = Memory.readMem<uintptr_t>(gameModule + dwLocalPlayer);
}

uintptr_t Functions::getLocalPlayer() {
    return Memory.readMem<uintptr_t>(gameModule + dwLocalPlayer);
}

int Functions::getHealth(uintptr_t entity) {
    return Memory.readMem<int>(entity + m_iHealth);
}

bool Functions::isAlive(uintptr_t entity) {
    if (getHealth(entity) > 0)
        return true;
    else
        return false;
}

bool Functions::isDefusing(uintptr_t entity) {
    return Memory.readMem<bool>(entity + m_bIsDefusing);
}

int Functions::getTeam(uintptr_t entity) {
    return Memory.readMem<int>(entity + m_iTeamNum);
}

BYTE Functions::getFlag(uintptr_t localPlayer) {
    return Memory.readMem<BYTE>(localPlayer + m_fFlags);
}

bool Functions::isGameRunning(){
    DWORD pid = 0;

    // Create toolhelp snapshot.
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32 process;
    ZeroMemory(&process, sizeof(process));
    process.dwSize = sizeof(process);

    if (Process32First(snapshot, &process))
    {
        do
        {
            if (strcmp((const char*)process.szExeFile, "csgo.exe"))
            {
               pid = process.th32ProcessID;
               break;
            }
        } while (Process32Next(snapshot, &process));
    }

    CloseHandle(snapshot);

    if (pid != 0)
        return true;
    else
        return false;


}

int Functions::getFlashDuration(uintptr_t localPlayer) {
    return Memory.readMem<int>(localPlayer + m_flFlashDuration);
}

void Functions::setFlashDuration(uintptr_t localPlayer, int duration) {
    Memory.writeMem<int>(localPlayer + m_flFlashDuration, duration);
}

uintptr_t Functions::getEntity(int index) {
    return Memory.readMem<uintptr_t>(gameModule + dwEntityList + index * 0x10);
}

bool Functions::checkIfScoped(uintptr_t entity) {
    return Memory.readMem<bool>(entity + m_bIsScoped);
}

char* Functions::getActiveWeapon(int entityWeapon)
{
    // should figure out a way to clean this up, 3 rpm calls for this is a bit much

    // also move this shit somewhere else, its ugly af
        switch(entityWeapon){
            case 1: return (char*)"Desert Eagle";
            case 2: return (char*)"Dual Berettas";
            case 3: return (char*)"Five-SeveN";
            case 4: return (char*)"Glock-18";
            case 7: return (char*)"AK-47";
            case 8: return (char*)"AUG";
            case 9: return (char*)"AWP";
            case 10: return (char*)"FAMAS";
            case 11: return (char*)"G3SG1";
            case 13: return (char*)"Galil AR";
            case 14: return (char*)"M249";
            case 16: return (char*)"M4A4";
            case 17: return (char*)"MAC-10";
            case 19: return (char*)"P90";
            case 20: return (char*)"Repulsor Device";
            case 23: return (char*)"MP5-SD";
            case 24: return (char*)"UMP-45";
            case 25: return (char*)"XM1014";
            case 26: return (char*)"PP-Bizon";
            case 27: return (char*)"MAG-7";
            case 28: return (char*)"Negev";
            case 29: return (char*)"Sawed-Off";
            case 30: return (char*)"Tec-9";
            case 31: return (char*)"Zeus x27";
            case 32: return (char*)"P2000";
            case 33: return (char*)"MP7";
            case 34: return (char*)"MP9";
            case 35: return (char*)"Nova";
            case 36: return (char*)"P250";
            case 37: return (char*)"Ballistic Shield";
            case 38: return (char*)"SCAR-20";
            case 39: return (char*)"SG 553";
            case 40: return (char*)"SSG 08";
            case 41: return (char*)"Knife";
            case 42: return (char*)"Knife";
            case 43: return (char*)"Flashbang";
            case 44: return (char*)"High Explosive Grenade";
            case 45: return (char*)"Smoke Grenade";
            case 46: return (char*)"Molotov";
            case 47: return (char*)"Decoy Grenade";
            case 48: return (char*)"Incendiary Grenade";
            case 49: return (char*)"C4 Explosive";
            case 50: return (char*)"Kevlar Vest";
            case 51: return (char*)"Kevlar + Helmet";
            case 52: return (char*)"Heavy Assault Suit";
            case 54: return (char*)"item_nvg";
            case 55: return (char*)"Defuse Kit";
            case 56: return (char*)"Rescue Kit";
            case 57: return (char*)"Medi-Shot";
            case 58: return (char*)"Music Kit";
            case 59: return (char*)"Knife";
            case 60: return (char*)"M4A1-S";
            case 61: return (char*)"USP-S";
            case 63: return (char*)"CZ75-Auto";
            case 64: return (char*)"R8 Revolver";
            case 68: return (char*)"Tactical Awareness Grenade";
            case 69: return (char*)"Bare Hands";
            case 70: return (char*)"Breach Charge";
            case 72: return (char*)"Tablet";
            case 75: return (char*)"Axe";
            case 76: return (char*)"Hammer";
            case 78: return (char*)"Wrench";
            case 80: return (char*)"Spectral Shiv";
            case 81: return (char*)"Fire Bomb";
            case 82: return (char*)"Diversion Device";
            case 83: return (char*)"Frag Grenade";
            case 84: return (char*)"Snowball";
            case 85: return (char*)"Bump Mine";
        }
    return (char*)"None";
}
