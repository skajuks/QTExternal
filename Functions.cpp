#include "Functions.h"
#include "offsets.hpp"
#include "windows.h"
#include <TlHelp32.h>

using namespace hazedumper::netvars;
using namespace hazedumper::signatures;

MemMan Memory;

int process_id = Memory.getProcess(L"csgo.exe");
uintptr_t gameModule = Memory.getModule(process_id, L"client.dll");
uintptr_t engineModule = Memory.getModule(process_id, L"engine.dll");
uintptr_t engineModulep = Functions::getClientState();

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
            if (process.szExeFile == (wchar_t*)"csgo.exe")
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
