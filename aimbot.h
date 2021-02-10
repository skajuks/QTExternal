#include <windows.h>
#include "Structs.h"
class Aim
{
public:
    static void AIMBOT();
    static void toggleAimbotOnKey(bool state);
    static void setFOVonSlider(int value);
    static void setSmoothOnSlider(float value);
    static void setRecoilControlPerc(float value);
    static void toggleSilentAim(bool state);
    static uintptr_t getClosestTeammate();
};

