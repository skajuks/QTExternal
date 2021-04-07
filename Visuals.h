#pragma once

#include <windows.h>

#include "Structs.h"

class Glow {
public:
    static bool glowEnabled();
    static void teamChamsStateChanged(int tChams[3]);
    static void enemyChamsStateChanged(int eChams[3]);
    static void brightnessStateChanged(bool state, float bright);
    static void setGlowAlpha(float value);
    static void setEnableHealthGlow(bool state);
    static void setGlowTeamColor(int r, int g, int b, bool lPlayerTeam);
    static void setGlowEnabled(bool state);
    static void ESP();
    static void StateChanged(int funct);
    static void colorChanged(int funct, int a, int r, int g, int b);
    static void setBrightness();
    static void ProcessEntityEnemy(const ClientInfo& ci, const Entity& e, uintptr_t glowObject);
    static void ProcessEntityTeam(const ClientInfo& ci, uintptr_t glowObject);
    static void ProcessD3D9Render(const ClientInfo& ci, const Entity& e, int index);
    static void ProcessTargetEntity(const ClientInfo& ci, uintptr_t glowObject);
};

