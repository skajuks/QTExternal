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

bool aimbot_toggle = false;
int aimKey = 0x01;
int bone_int = 8;
int aimfov = 20;
float smooth = 1.f;
float recoil_perc = 1.f;
bool enable_silentaim = false;

char* pistols[] ={0,0,0,0,0};   // add for every gun

// ui fucntions

void Aim::toggleAimbotOnKey(bool state){
    aimbot_toggle = state;
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

VECTOR2 Aim::getClosestEntity(const Entity &Entity_local, const ClientInfo &ci, const ClientInfo &local_ci){

    VECTOR3 recoil = Memory.readMem<VECTOR3>(local_ci.entity + m_aimPunchAngle);   // gets recoil vector and counteracts it
    recoil *= (recoil_perc / 90.f) * 2.f;

    VECTOR3 vecOrigin = Entity_local.vecOrigin;
    VECTOR3 entBone = Math::getBoneMatrix(ci.entity, bone_int);
    VECTOR3 pAngle = Math::PlayerAngles();
    pAngle.z = Entity_local.vecViewOffset.z;    // pAngle.z = Memory.readMem<float>(localPlayer + m_vecViewOffset + 0x8);
    vecOrigin.z += pAngle.z;

    VECTOR3 angleTo = Math::CalcAngle(Entity_local.vecOrigin, entBone);
    return Math::CalcDistance(pAngle.x, pAngle.y, angleTo.x, angleTo.y);
}


void Aim::getClosestEnemyByAngle(Entity* Entity,ClientInfo* pEntity){               //redo this function, doesnt need while loop and toggle check
    if(aimbot_toggle){
        int entityIndex = 1;    // skips localplayer

        float oldDistancex = std::numeric_limits<float>::max();;
        float oldDistancey = std::numeric_limits<float>::max();;
        int entityWeapon;
        ClientInfo* target;

        VECTOR3 entBone;
        VECTOR3 pAngle;
        VECTOR3 recoil;

        VECTOR3 targetBones;
        VECTOR3 old_recoil{0.f, 0.f, 0.f};

        do{
            if(entityIndex >= 64)
                break;

            if(Entity[entityIndex].health && pEntity[entityIndex].entity && Entity[entityIndex].team != Entity[0].team){

                recoil = Memory.readMem<VECTOR3>(pEntity[0].entity + m_aimPunchAngle);   // gets recoil vector and counteracts it
                recoil *= (recoil_perc / 90.f) * 2.f;

                int weaponEnt = Memory.readMem<int>(gameModule + dwEntityList + ((Memory.readMem<int>(pEntity[0].entity + m_hActiveWeapon) & 0xFFF) - 1) * 0x10);
                if(weaponEnt){
                    entityWeapon = Memory.readMem<int>(weaponEnt + m_iItemDefinitionIndex);     // gets weapon entity ID, currently returns 0 [ recheck ]
                }

                entBone = Math::getBoneMatrix(pEntity->entity, bone_int);
                pAngle = Math::PlayerAngles();
                pAngle.z = Entity[0].vecViewOffset.z;    // pAngle.z = Memory.readMem<float>(localPlayer + m_vecViewOffset + 0x8);
                Entity[0].vecOrigin.z += pAngle.z;

                VECTOR3 angleTo = Math::CalcAngle(Entity[0].vecOrigin, entBone);
                auto newDistance = Math::CalcDistance(pAngle.x, pAngle.y, angleTo.x, angleTo.y);
                if(newDistance.x < oldDistancex && newDistance.y < oldDistancey && newDistance.x <= aimfov && newDistance.y <= aimfov){
                    oldDistancex = newDistance.x;
                    oldDistancey = newDistance.y;
                    targetBones = entBone;
                    target = pEntity;
                }

            entityIndex++;

            }
        } while(pEntity->nextEntity);

        if(target != NULL){
            VECTOR3 AimAngle = Math::CalcAngle(Entity[0].vecOrigin, targetBones);
            AimAngle = Math::Smooth(smooth, pAngle, AimAngle - recoil);
            Math::normalizeAngles(&AimAngle.x , &AimAngle.y);
            VECTOR2 readyAngles = {AimAngle.x, AimAngle.y};
            if(!in_array(noAimbotEntityArray, 11, entityWeapon)){
                if(enable_silentaim)
                    doSilentAim(readyAngles, true);
                else
                   Memory.writeMem<VECTOR2>(engineModulep + dwClientState_ViewAngles, readyAngles);
            }
        }
    }
}
















