#include "aimbot.h"
#include "Functions.h"
#include "offsets.hpp"
#include "csmath.h"


#undef max

/*
        to do list:

        make a check for weapon in hand and ajust recoil accordingly
        make a check if weapon is scoped
        remove recoil for snipers
        shoot only if target can be killed ( no dm protection)
        add closest entity by fov
        add triggerbot
*/


using namespace hazedumper::signatures;
using namespace hazedumper::netvars;

int bone_int = 8;
int aimfov = 20;
float smooth = 1.f;
float recoil_perc = 1.f;
bool enable_silentaim = false;

char* pistols[] ={0,0,0,0,0};   // add for every gun

// ui fucntions

void Aim::enableJumpShot(){
}

void Aim::setFOVonSlider(int value){
    aimfov = value;
}

void Aim::setSmoothOnSlider(float value){
    smooth = value;
}

void Aim::setRecoilControlPerc(float value){
    recoil_perc = value;
}

void Aim::toggleSilentAim(bool state){
    enable_silentaim = state;
}

void doSilentAim(VECTOR2 angles, bool CanShoot){
    CInput Input;
    DWORD clientState;

    Input = Memory.readMem<CInput>(gameModule + dwInput);
    clientState = Functions::getClientState();

    Memory.writeMem<BYTE>(engineModule + dwbSendPackets, 0); //disable packet send

    int desiredCMD;
    desiredCMD = Memory.readMem<int>(clientState + clientstate_last_outgoing_command);
    desiredCMD += 2; // +2 for incomming one

    DWORD incommingUserCMD = Input.Commands + (desiredCMD % 150) * sizeof(CUserCmd);
    DWORD currentUserCMD = Input.Commands + ((desiredCMD - 1) % 150) * sizeof(CUserCmd);
    DWORD verifiedCurUserCMD = Input.VerifiedCommands + ((desiredCMD - 1) % 150) * sizeof(CVerifiedUserCmd);

    int CMDNumber = NULL;
    while(CMDNumber < desiredCMD)
        CMDNumber = Memory.readMem<int>(incommingUserCMD + 0x04);

    CUserCmd cmd; // read the usercmd
    Memory.readMemTo<CUserCmd>(currentUserCMD, &cmd);

    cmd.ViewAngles.x = angles.x;
    cmd.ViewAngles.y = angles.y;

    if(CanShoot) // triggers shooting if set to tru
        cmd.Buttons |= true;

    Memory.writeMemFrom<CUserCmd>(currentUserCMD, &cmd);
    Memory.writeMemFrom<CUserCmd>(verifiedCurUserCMD, &cmd);

    Memory.writeMem<BYTE>(engineModule + dwbSendPackets, 1); //restore packet sending :)
}

int noAimbotEntityArray[] = {41,42,43,44,45,46,47,48,49,57,59};

bool in_array(const int store[], const int storeSize, const int querry){
    for(short int i=0; i<storeSize; i++){
        if(store[i] == querry)
            return true;
    }
    return false;
}

ClientInfo* Aim::getClosestTeammate(Entity* localPlayer, ClientInfo* pEntity){

    // use this to get closest team entity for blockbot

    float oldDistancex = std::numeric_limits<float>::max();;
    float oldDistancey = std::numeric_limits<float>::max();;
    ClientInfo* target;

    VECTOR3 entBone = Math::getBoneMatrix(pEntity->entity, bone_int);
    VECTOR3 pAngle = Math::PlayerAngles();
    pAngle.z = localPlayer->vecViewOffset.z;    // pAngle.z = Memory.readMem<float>(localPlayer + m_vecViewOffset + 0x8);
    localPlayer->vecOrigin.z += pAngle.z;

    VECTOR3 angleTo = Math::CalcAngle(localPlayer->vecOrigin, entBone);
    auto newDistance = Math::CalcDistance(pAngle.x, pAngle.y, angleTo.x, angleTo.y);

    if(newDistance.x < oldDistancex && newDistance.y < oldDistancey){
        oldDistancex = newDistance.x;
        oldDistancey = newDistance.y;
        target = pEntity;
    }
    if(target)
        return target;
    return NULL;
}

float Aim::getClosestEntityByDistance(const Entity &Entity_local, const Entity &Entity_target){
    VECTOR3 vecOrigin = Entity_local.vecOrigin;
    VECTOR3 vecOriginEntity = Entity_target.vecOrigin;
    VECTOR3 delta = vecOrigin - vecOriginEntity;
    return sqrt(delta.x * delta.x + delta.y * delta.y + delta.z * delta.z);
}

float Aim::getClosestEntityByDistance(const Entity &Entity_local, VECTOR3 Entity_target){
    VECTOR3 vecOrigin = Entity_local.vecOrigin;
    VECTOR3 delta = vecOrigin - Entity_target;
    return sqrt(delta.x * delta.x + delta.y * delta.y + delta.z * delta.z);
}

VECTOR2 Aim::getClosestEntity(const Entity &Entity_local, const ClientInfo &ci){

    VECTOR3 vecOrigin = Entity_local.vecOrigin;
    VECTOR3 entBone = Math::getBoneMatrix(ci.entity, bone_int);
    VECTOR3 pAngle = Math::PlayerAngles();
    pAngle.z = Entity_local.vecViewOffset.z;    // pAngle.z = Memory.readMem<float>(localPlayer + m_vecViewOffset + 0x8);
    vecOrigin.z += pAngle.z;

    VECTOR3 angleTo = Math::CalcAngle(vecOrigin, entBone);
    VECTOR2 finalAngle = Math::CalcDistance(pAngle.x, pAngle.y, angleTo.x, angleTo.y);
    //printf("FINAL_ANGLE = %f %f\n", finalAngle.x ,finalAngle.y);
    return finalAngle;
}

aimbotVariables Aim::executeAimbot(const ClientInfo &target, const Entity &Entity_local, const ClientInfo &local_ci){
    int entityWeapon;
    aimbotVariables variables;

    VECTOR3 recoil = Memory.readMem<VECTOR3>(local_ci.entity + m_aimPunchAngle);   // gets recoil vector and counteracts it
    recoil *= (recoil_perc / 90.f) * 2.f;

    int weaponEnt = Memory.readMem<int>(gameModule + dwEntityList + ((Memory.readMem<int>(local_ci.entity + m_hActiveWeapon) & 0xFFF) - 1) * 0x10);
    if(weaponEnt){
        entityWeapon = Memory.readMem<int>(weaponEnt + m_iItemDefinitionIndex);     // gets weapon entity ID, currently returns 0 [ recheck ]
    }
    VECTOR3 vecOrigin = Entity_local.vecOrigin;
    VECTOR3 pAngle = Math::PlayerAngles();
    pAngle.z = Entity_local.vecViewOffset.z;    // pAngle.z = Memory.readMem<float>(localPlayer + m_vecViewOffset + 0x8);
    vecOrigin.z += pAngle.z;

    VECTOR3 AimAngle = Math::CalcAngle(vecOrigin, Math::getBoneMatrix(target.entity, 8));   // 8 head   // 6 chest  // 5 lower chest // 9 neck //  3 pelvis
    AimAngle = Math::Smooth(smooth, pAngle, AimAngle - recoil);
    Math::normalizeAngles(&AimAngle.x , &AimAngle.y);
    VECTOR2 readyAngles = {AimAngle.x, AimAngle.y};

    if(!in_array(noAimbotEntityArray, 11, entityWeapon)){
            if(enable_silentaim)
                doSilentAim(readyAngles, true);
            else
               Memory.writeMem<VECTOR2>(engineModulep + dwClientState_ViewAngles, readyAngles);

    }
    variables.entityWeapon = entityWeapon;
    variables.AimAngle = AimAngle;
    variables.recoil = recoil;
    return variables;
}
