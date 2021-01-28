#pragma once
#include "MemMan.h"
#include "windows.h"

extern MemMan Memory;
extern int process_id;
extern uintptr_t gameModule;
extern uintptr_t engineModule;
extern uintptr_t engineModulep;

class Functions{
public:
    static uintptr_t getClientState();
    static void checkForLocalPlayer();
    static uintptr_t getLocalPlayer();
    static int getHealth(uintptr_t entity);
    static bool isAlive(uintptr_t entity);
    static bool isDefusing(uintptr_t entity);
    static int getTeam(uintptr_t entity);
    static BYTE getFlag(uintptr_t localPlayer);
    static bool isGameRunning();
    static int getFlashDuration(uintptr_t localPlayer);
    static void setFlashDuration(uintptr_t localPlayer, int duration);
    static uintptr_t getEntity(int index);

};extern Functions fun;
