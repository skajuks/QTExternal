#ifndef LOCALPLAYER_H
#define LOCALPLAYER_H
#include "csmath.h"
#include "Functions.h"
#include "Structs.h"

class LocalPlayer
{
public:
    static float getSpeed(const Entity& local);
    static bool inGame();
    static int getActiveWeapon(const ClientInfo& ci);
};

#endif // LOCALPLAYER_H
