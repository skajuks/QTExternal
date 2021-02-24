#include <windows.h>
#include "Structs.h"
class Aim
{
public:
    static ClientInfo* getClosestTeammate(Entity* localPlayer, ClientInfo* pEntity);
    static void getClosestEnemyByAngle(Entity* Entity, ClientInfo* pEntity);
    static void AIMBOT();
    static void toggleAimbotOnKey(bool state);
    static void setFOVonSlider(int value);
    static void setSmoothOnSlider(float value);
    static void setRecoilControlPerc(float value);
    static void toggleSilentAim(bool state);
    static VECTOR2 getClosestEntity(const Entity &Entity_local, const ClientInfo &ci);
    static void executeAimbot(const ClientInfo &target, const Entity &Entity_local, const ClientInfo &local_ci);
};

