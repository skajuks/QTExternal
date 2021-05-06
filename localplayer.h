#ifndef LOCALPLAYER_H
#define LOCALPLAYER_H
#include "offsets.hpp"
#include "csmath.h"
#include "Functions.h"
#include "Structs.h"

using namespace hazedumper::netvars;
using namespace hazedumper::signatures;


class LocalPlayer
{
public:
    static float getSpeed(const Entity& local);
    static bool inGame();
    static int getActiveWeapon(const ClientInfo& ci);
};

#endif // LOCALPLAYER_H
