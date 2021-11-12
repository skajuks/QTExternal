#include "Functions.h"
#include "windows.h"
#include "csmath.h"
#include <TlHelp32.h>
#include <iostream>
#include <thread>

MemMan Memory;
NetVars Netv;

int process_id = Memory.getProcess(L"csgo.exe");
uintptr_t gameModule = Memory.getModule(process_id, L"client.dll");
uintptr_t engineModule = Memory.getModule(process_id, L"engine.dll");
uintptr_t engineModulep = Functions::getClientState();

Offsets pOffsets = Memory.getOffsets(process_id); // init offsets from sigs
netVarStr pNetVars = Netv.dumpNetvars();

void Functions::clientCmd_Unrestricted(const char* command){    // Function used to access in-game console commands
    if(!Memory.getAlloc()) return;
    static uintptr_t alloc = reinterpret_cast<uintptr_t>(Memory.getAlloc());
    static bool injected = false;
    if(!injected){
        BYTE Shellcode[15 + 128] = { 0x6A, 0x00,					// push   0x0
                                            0x68, 0x00, 0x00, 0x00, 0x00,	// push   (pointer to command)
                                            0xB8, 0x00, 0x00, 0x00, 0x00,	// mov    eax,(ClientCmd_Un)
                                            0xFF, 0xD0,						// call   eax
                                            0xC3 };							// ret
                                                                            // (command)
        *reinterpret_cast<uintptr_t*>(&Shellcode[3]) = alloc + 15;
        *reinterpret_cast<uintptr_t*>(&Shellcode[8]) = engineModule + pOffsets.clientCmd_Unrestricted;
        strcpy_s(reinterpret_cast<char*>(&Shellcode[15]), 128, command);
        Memory.write<BYTE[15 + 128]>(alloc, Shellcode);
        injected = true;
    } else {
        WriteProcessMemory(Memory.handle, reinterpret_cast<LPVOID>(alloc + 15), command, strlen(command) + 1,0);
    }
    Memory.createThread(alloc);
}

void Functions::moveRight(){
    clientCmd_Unrestricted("-moveleft");
    Sleep(1);
    clientCmd_Unrestricted("moveright");
}

void Functions::moveLeft(){
    clientCmd_Unrestricted("-moveright");
    Sleep(1);
    clientCmd_Unrestricted("moveleft");
}

void Functions::moveClearY(){
    clientCmd_Unrestricted("-moveright");
    Sleep(1);
    clientCmd_Unrestricted("-moveleft");
}

void Functions::moveForward(){
    clientCmd_Unrestricted("+forward");
}

void Functions::moveClearX(){
    clientCmd_Unrestricted("-forward");
}

uintptr_t Functions::getClientState() {
    return Memory.readMem<uintptr_t>(engineModule + pOffsets.dwClientState);
}

bool Functions::isDefusing(uintptr_t entity) {
    return Memory.readMem<bool>(entity + pNetVars.m_bIsDefusing);
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
    return false;
}

void Functions::sideSpeed(float value) {
    Memory.writeMem<uint32_t>(Memory.readMem<uint32_t>(gameModule + pOffsets.cl_sidespeed), (*(uint32_t*)&value) ^ pOffsets.xor_cl_sidespeed);
}

void Functions::forwardSpeed(int value) {
    Memory.writeMem<uint32_t>(Memory.readMem<uint32_t>(gameModule + pOffsets.cl_forwardspeed), (*(uint32_t*)&value) ^ pOffsets.xor_cl_forwardspeed);
}

void Functions::setFlashDuration(uintptr_t localPlayer, float duration) {
    Memory.writeMem<float>(localPlayer + pNetVars.m_flFlashDuration, duration);
}

bool Functions::checkPlayerSpottedByMask(uintptr_t entity, uintptr_t localPlayer){
    uintptr_t spotted_by_mask = Memory.readMem<uintptr_t>(entity + pNetVars.m_bSpottedByMask);
    //if(spotted_by_mask & (1 << local_player_id)) return true
    // return false
    return true;
}

uintptr_t Functions::getEntity(int index) {
    return Memory.readMem<uintptr_t>(pOffsets.dwEntityList + index * 0x10);
}

bool Functions::checkIfScoped(uintptr_t entity) {
    return Memory.readMem<bool>(entity + pNetVars.m_bIsScoped);
}

void Functions::assignColorFromStruct(ColorRGB &color, GlowObject &glow){
    glow.red = color.r;
    glow.green = color.g;
    glow.blue = color.b;
    glow.alpha = 255;
}

void Functions::getPlayerInfo(int &index, player_info &player, uintptr_t& clientState){
    uintptr_t uinfoTable = Memory.readMem<uintptr_t>(clientState + pOffsets.dwClientState_PlayerInfo);
    uintptr_t items = Memory.readMem<std::uintptr_t>(Memory.readMem<uintptr_t>(uinfoTable + 0x40) + 0xC);
    Memory.readMemTo<player_info>(Memory.readMem<uintptr_t>((items + 0x28) + (index * 0x34)), &player);   // read player struct
}

char* Functions::getActiveWeapon(int entityWeapon)
{
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
