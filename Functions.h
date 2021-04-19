#pragma once
#include "MemMan.h"
#include "windows.h"

extern MemMan Memory;
extern int process_id;
extern uintptr_t gameModule;
extern uintptr_t engineModule;
extern uintptr_t engineModulep;
extern Offsets pOffsets;

class Functions{
public:
    // moves
    static void moveRight();
    static void moveLeft();
    static void moveClearY();
    static void moveForward();
    static void moveClearX();

    static void loadSkybox(const char* skyname);
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
    static char* getActiveWeapon(int entityWeapon);
    static bool checkIfScoped(uintptr_t entity);
    static void forwardSpeed(int value);
    static void sideSpeed(float value);
    static void clientCmd_Unrestricted(const char* command);

};extern Functions fun;
