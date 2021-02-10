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
    cmd = Memory.readMem<CUserCmd>(currentUserCMD);

    cmd.ViewAngles.x = angles.x;
    cmd.ViewAngles.y = angles.y;

    if(CanShoot) // triggers shooting if set to tru
        cmd.Buttons |= true;

    Memory.writeMem<CUserCmd>(currentUserCMD, cmd);
    Memory.writeMem<CUserCmd>(verifiedCurUserCMD, cmd);

    Memory.writeMem<BYTE>(engineModule + dwbSendPackets, 1); //restore packet sending :)
}


VECTOR3 pPos;
VECTOR3 entBone;
VECTOR3 pAngle;
VECTOR2 normalAngles;
VECTOR3 targetBones;
VECTOR3 old_recoil{0.f, 0.f, 0.f};
int noAimbotEntityArray[] = {41,42,43,44,45,46,47,48,49,57,59};

bool in_array(const int store[], const int storeSize, const int querry){
    for(size_t i=0; i<storeSize; i++){
        if(store[i] == querry)
            return true;
    }
    return false;
}

uintptr_t Aim::getClosestTeammate(){               //redo this function, doesnt need while loop and toggle check

    uintptr_t localPlayer = Functions::getLocalPlayer();
    float oldDistancex = std::numeric_limits<float>::max();;
    float oldDistancey = std::numeric_limits<float>::max();;
    VECTOR3 angleTo;
    VECTOR3 AimAngle;
    uintptr_t target = NULL;

    for(short int i = 0; i < 32; i++){
        uintptr_t entity = Functions::getEntity(i);
        if(Functions::getTeam(entity) == Functions::getTeam(localPlayer) && Functions::isAlive(entity)){
            pPos = Math::PlayerPos(localPlayer);
            entBone = Math::getBoneMatrix(entity, bone_int);
            pAngle = Math::PlayerAngles();
            pAngle.z = Memory.readMem<float>(localPlayer + m_vecViewOffset + 0x8);
            pPos.z += pAngle.z;
            angleTo = Math::CalcAngle(pPos, entBone);
            auto newDistance = Math::CalcDistance(pAngle.x, pAngle.y, angleTo.x, angleTo.y);
            if(newDistance.x < oldDistancex && newDistance.y < oldDistancey && newDistance.x <= aimfov && newDistance.y <= aimfov){
                oldDistancex = newDistance.x;
                oldDistancey = newDistance.y;
                target = entity;
            }
        }
    }
    if(target != NULL)
        return target;
    return NULL;
}

void getClosestEnemyByAngle(){               //redo this function, doesnt need while loop and toggle check

    uintptr_t localPlayer = Functions::getLocalPlayer();
    float oldDistancex = std::numeric_limits<float>::max();;
    float oldDistancey = std::numeric_limits<float>::max();;
    VECTOR3 angleTo;
    VECTOR3 AimAngle;
    VECTOR2 readyAngles;
    int entityWeapon;
    uintptr_t target = NULL;
    if(GetAsyncKeyState(aimKey)){
        VECTOR3 recoil = Memory.readMem<VECTOR3>(localPlayer + m_aimPunchAngle);
        recoil *= (recoil_perc / 90.f) * 2.f;
        int weaponId = Memory.readMem<int>(localPlayer + m_hActiveWeapon);
        int weaponEnt = Memory.readMem<int>(gameModule + dwEntityList + ((weaponId & 0xFFF) - 1) * 0x10);
        if(weaponEnt != NULL){
             entityWeapon = Memory.readMem<int>(weaponEnt + m_iItemDefinitionIndex);
        for(short int i = 0; i < 32; i++){
            uintptr_t entity = Functions::getEntity(i);
            if(Functions::getTeam(entity) != Functions::getTeam(localPlayer) && Functions::isAlive(entity)){
                pPos = Math::PlayerPos(localPlayer);
                entBone = Math::getBoneMatrix(entity, bone_int);
                pAngle = Math::PlayerAngles();
                pAngle.z = Memory.readMem<float>(localPlayer + m_vecViewOffset + 0x8);
                pPos.z += pAngle.z;
                angleTo = Math::CalcAngle(pPos, entBone);
                auto newDistance = Math::CalcDistance(pAngle.x, pAngle.y, angleTo.x, angleTo.y);
                if(newDistance.x < oldDistancex && newDistance.y < oldDistancey && newDistance.x <= aimfov && newDistance.y <= aimfov){
                    oldDistancex = newDistance.x;
                    oldDistancey = newDistance.y;
                    targetBones = entBone;
                    target = entity;
                }
            }
        }
        if(target != NULL){
            AimAngle = Math::CalcAngle(pPos, targetBones);
            AimAngle = Math::Smooth(smooth, pAngle, AimAngle - recoil);
            Math::normalizeAngles(&AimAngle.x , &AimAngle.y);
            readyAngles = {AimAngle.x, AimAngle.y};
            if(!in_array(noAimbotEntityArray, 11, entityWeapon)){
                if(enable_silentaim)
                    doSilentAim(readyAngles, true);
                else
                    Memory.writeMem<VECTOR2>(engineModulep + dwClientState_ViewAngles, readyAngles);

            }

        }
       }
    }
}

void Aim::AIMBOT(){
    while(true){
        if(aimbot_toggle){
            //if(GetKeyState(VK_HOME) & 1)
            getClosestEnemyByAngle();

        }
        Sleep(1);
    }
}


















