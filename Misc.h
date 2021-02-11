#pragma once
#include "Structs.h"

class Misc
{
public:
    static void setBhop(const Entity& local_entity);
    static bool bhopEnabled();
    static void setBhopEnabled(bool state);
    static void setEnabledNoFlash(bool state);
    static void setNoFlash(const ClientInfo& localPlayer);
    static void setNightmodeAmount(const ClientInfo& localPlayer, float amount);
    static void changeFov(const ClientInfo& localPlayer, int fov);
};
