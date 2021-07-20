#include "localplayer.h"

float LocalPlayer::getSpeed(const Entity& local){
    return Math::returnVelocity(local);
}

int LocalPlayer::getActiveWeapon(const ClientInfo &ci){
    int weapon_handle = Memory.readMem<int>(ci.entity + pNetVars.m_hActiveWeapon) & 0xFFF;
    return Memory.readMem<int>(Memory.readMem<int>(pOffsets.dwEntityList + (weapon_handle - 1) * 0x10) + pNetVars.m_iItemDefinitionIndex);
}
