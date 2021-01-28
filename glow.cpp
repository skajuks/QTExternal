#include "glow.h"
#include "Functions.h"
#include "offsets.hpp"
#include "Signatures.hpp"
#include "NetVars.hpp"

using namespace hazedumper::netvars;
using namespace hazedumper::signatures;

Chams clr_team;
Chams clr_enemy;
uintptr_t glowObject;

bool glow_enabled = 0;
bool enableHealthGlow = 0;

bool alpha = 1.f;
// team glows
int TeamR = 0;
int TeamG = 0;
int TeamB = 255;

//enemy glows
int ETeamR = 0;
int ETeamG = 0;
int ETeamB = 255;


bool Glow::glowEnabled(){
    if (glow_enabled)
        return true;
    else
        return false;
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
        TeamR = r;
        TeamG = g;
        TeamB = b;
    }else{
        ETeamR = r;
        ETeamG = g;
        ETeamB = b;
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
    clr_enemy.green = 225;
    clr_enemy.red = 0;

    clr_team.red = 225;
    clr_team.green = 225;
    clr_team.blue = 0;

    float brightness = 100.0f;
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

void Glow::ESP() {
    setChams();
    while (true) {
        if (!glowEnabled()) {
            uintptr_t localPlayer = Functions::getLocalPlayer();
            handleGlow(localPlayer);
        }
        Sleep(1);
    }
}
