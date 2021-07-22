#include <windows.h>
#include "Structs.h"
class Aim
{
public:
    static void getClosestEnemyByAngle(Entity* Entity, ClientInfo* pEntity);
    static void AIMBOT();
    static void toggleAimbotOnKey(bool state);
    static void setFOVonSlider(int value);
    static void setSmoothOnSlider(float value);
    static void setRecoilControlPerc(float value);
    static void toggleSilentAim(bool state);
    static VECTOR2 getClosestEntity(const Entity &Entity_local, const ClientInfo &ci);
    static aimbotVariables executeAimbot(const ClientInfo &target, const Entity &Entity_local, const ClientInfo &local_ci);
    static void enableJumpShot();
    static float getClosestEntityByDistance(const Entity &Entity_local, VECTOR3 Entity_target);
    static float getClosestEntityByDistance(const Entity &Entity_local, const Entity &Entity_target);
    static float getClosestEntityByDistance(VECTOR3 local, VECTOR3 target);
};

