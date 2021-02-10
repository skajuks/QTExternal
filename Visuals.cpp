#include "Visuals.h"
#include "Functions.h"
#include "offsets.hpp"
#include "Signatures.hpp"
#include "NetVars.hpp"
#include "Structs.h"

using namespace hazedumper::netvars;
using namespace hazedumper::signatures;

Chams clr_team;
Chams clr_enemy;

uintptr_t glowObject;

bool glow_enabled = false;
bool enableHealthGlow = false;
bool call_brightness_on_loop = false;
float alpha = 1.f;

float brightness = 100.f;

// team glows
BYTE TeamR = 0;
BYTE TeamG = 0;
BYTE TeamB = 255;

//enemy glows
BYTE ETeamR = 0;
BYTE ETeamG = 0;
BYTE ETeamB = 255;

bool Glow::glowEnabled(){
    if(glow_enabled)
        return true;
    else
        return false;
}


void Glow::teamChamsStateChanged(int tChams[3]){
    clr_team.red = (BYTE)tChams[0];
    clr_team.green = (BYTE)tChams[1];
    clr_team.blue = (BYTE)tChams[2];
}


void Glow::enemyChamsStateChanged(int eChams[3]){
    clr_enemy.red = (BYTE)eChams[0];
    clr_enemy.green = (BYTE)eChams[1];
    clr_enemy.blue = (BYTE)eChams[2];
}

void Glow::brightnessStateChanged(bool state, float bright){
    brightness = bright;
    call_brightness_on_loop = state;
}

void Glow::setGlowEnabled(bool state){
    glow_enabled = state;
}
void Glow::setEnableHealthGlow(bool state){
    enableHealthGlow = state;
}

void Glow::setGlowAlpha(float value){
    alpha = value;
}

void Glow::setGlowTeamColor(int r, int g, int b, bool lPlayerTeam){
    if(lPlayerTeam){
        TeamR = (BYTE)r;
        TeamG = (BYTE)g;
        TeamB = (BYTE)b;
    }else{
        ETeamR = (BYTE)r;
        ETeamG = (BYTE)g;
        ETeamB = (BYTE)b;
    }
}

static GlowObject setGlowColor(GlowObject glow, int health, uintptr_t entity) {
    if (Functions::isDefusing(entity)) {
        glow.red = 255;
        glow.blue = 255;
        glow.green = 255;
    }
    else {
        if(enableHealthGlow){
            glow.red = health * -0.01 + 1;
            glow.green = health * 0.01;
        } else {
            glow.red = ETeamR;
            glow.green = ETeamG;
            glow.blue = ETeamB;
        }
    }
    glow.alpha = alpha;
    glow.renderWhenOccluded = true;
    glow.renderWhenUnoccluded = false;
    return glow;
}

static void setEnemyGlow(uintptr_t entity, int glowIndex) {
    GlowObject eGlow;
    eGlow = Memory.readMem<GlowObject>(glowObject + (glowIndex * 0x38));
    int health = Memory.readMem<int>(entity + m_iHealth);
    eGlow = setGlowColor(eGlow, health, entity);
    Memory.writeMem<GlowObject>(glowObject + (glowIndex * 0x38), eGlow);
}

static void setTeamGlow(uintptr_t entity, int glowIndex) {
    GlowObject tGlow;
    tGlow = Memory.readMem<GlowObject>(glowObject + (glowIndex * 0x38));
    tGlow.blue = TeamB;
    tGlow.red = TeamR;
    tGlow.green = TeamG;
    tGlow.alpha = alpha;
    tGlow.renderWhenOccluded = true;
    tGlow.renderWhenUnoccluded = false;
    Memory.writeMem<GlowObject>(glowObject + (glowIndex * 0x38), tGlow);
}

static void setChams() {
    clr_enemy.blue = 255;
    clr_enemy.green = 255;
    clr_enemy.red = 0;

    clr_team.red = 255;
    clr_team.green = 255;
    clr_team.blue = 0;
}

static void setBrightness() {
    int ptr = Memory.readMem<int>(engineModule + model_ambient_min);
    int xorptr = *(int*)&brightness ^ ptr;
    Memory.writeMem<int>(engineModule + model_ambient_min, xorptr);
}

static void handleGlow(uintptr_t localPlayer) {
    glowObject = Memory.readMem<uintptr_t>(gameModule + dwGlowObjectManager);
    int myTeam = Functions::getTeam(localPlayer);
    for (short int i = 1; i < 32; i++) {
        uintptr_t entity = Memory.readMem<uintptr_t>(gameModule + dwEntityList + i * 0x10);
        if (entity != 0 && Functions::isAlive(entity)) {
            int glowIndex = Memory.readMem<int>(entity + m_iGlowIndex);
            int entityteam = Functions::getTeam(entity);
            if (myTeam == entityteam) {
                setTeamGlow(entity, glowIndex);
                Memory.writeMem<Chams>(entity + m_clrRender, clr_team);
            }
            else {
                setEnemyGlow(entity, glowIndex);
                Memory.writeMem<Chams>(entity + m_clrRender, clr_enemy);
            }
        }
    }
}

void Glow::ProcessEntity(const ClientInfo& ci, const Entity& e, uintptr_t glowObject) {
    int glowIndex = Memory.readMem<int>(ci.entity + m_iGlowIndex);
    GlowObject eGlow;
    Memory.readMemTo<GlowObject>(glowObject + (glowIndex * 0x38), &eGlow);

    eGlow = setGlowColor(eGlow, e.health, ci.entity);

    Memory.writeMemFrom<GlowObject>(glowObject + (glowIndex * 0x38), &eGlow);
}

void Glow::ESP() {
    setChams();
    setBrightness();        // initial calls to set them for the first time using default values

    while (true) {
        if (glow_enabled) {
            uintptr_t localPlayer = Functions::getLocalPlayer();
            handleGlow(localPlayer);
            if (call_brightness_on_loop){
                setBrightness();
                call_brightness_on_loop = false;
            }
        }
        Sleep(1);
    }
}
