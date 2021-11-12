#include "Visuals.h"
#include "Functions.h"
#include "Structs.h"
#include "paint.h"

Chams clr_team = {255,255,0};
Chams clr_enemy = {0,255,255};

Paint Paint();

bool glow_enabled = false;
bool enableHealthGlow = true;
bool call_brightness_on_loop = false;
float alpha = 1.f;

float brightness = 100.f;

// team glows
BYTE TeamR = 255;
BYTE TeamG = 0;
BYTE TeamB = 0;

//enemy glows
BYTE ETeamR = 0;
BYTE ETeamG = 0;
BYTE ETeamB = 255;

// toggles
bool master_esp_toggle = false;
bool boxes_enabled = false;
bool snapl_enabled = false;
bool weapon_health_enabled = false;
bool boxes_by_health_enabled = false;

// init colors
Color snaplineColor = {255,100,100,100};
Color espBoxColor = {100,100,100,255};
Color crosshairColor = {255,50,205,50};

void Glow::StateChanged(int funct){
    switch(funct){
    case 1: master_esp_toggle = !master_esp_toggle;                 // toggle esp master
    case 2: boxes_enabled = !boxes_enabled;                         // toggle esp boxes
    case 3: weapon_health_enabled = !weapon_health_enabled;         // toggle esp health, active weapon and player name
    case 4: snapl_enabled = !snapl_enabled;                         // toggle snaplines
    case 5: boxes_by_health_enabled = !boxes_by_health_enabled;     // toggle esp draw by health
    }
}

void Glow::colorChanged(int funct, int a, int r, int g, int b){
    switch(funct){
    case 1: snaplineColor.a = a; snaplineColor.r = r; snaplineColor.g = g; snaplineColor.b = b;  // change snapline color
    case 2: espBoxColor.a = a; espBoxColor.r = r; espBoxColor.g = g; espBoxColor.b = b;          // change esp box color
    }
}

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
    setBrightness();
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

static GlowObject setGlowColor(GlowObject glow, int health, uintptr_t entity, bool targetindex) {
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
        if(targetindex){
            glow.red = 255;
            glow.green = 255;
            glow.blue = 0;
        }
    }
    glow.alpha = alpha;
    glow.renderWhenOccluded = true;
    glow.renderWhenUnoccluded = false;
    return glow;
}

void Glow::setBrightness() {
    int ptr = Memory.readMem<int>(pOffsets.model_ambient_min);
    int xorptr = *(int*)&brightness ^ ptr;
    Memory.writeMem<int>(pOffsets.model_ambient_min, xorptr);
}

void Glow::ProcessEntityTeam(const ClientInfo& ci, uintptr_t& glowObject, bool targetindex) {
    if(glow_enabled){
        int glowIndex = Memory.readMem<int>(ci.entity + pNetVars.m_iGlowIndex);
        GlowObject tGlow;
        Memory.readMemTo<GlowObject>(glowObject + (glowIndex * 0x38), &tGlow);
        //tGlow = {{}, (float)TeamR, (float)TeamG, (float)TeamB, alpha, {}, true, false};
        tGlow.red = TeamR;
        tGlow.green = TeamG;
        tGlow.blue = TeamB;
        if(targetindex){
            tGlow.red = 255;
            tGlow.green = 255;
            tGlow.blue = 0;
        }
        tGlow.alpha = alpha;
        tGlow.renderWhenOccluded = true;
        tGlow.renderWhenUnoccluded = false;
        Memory.writeMemFrom<GlowObject>(glowObject + (glowIndex * 0x38), &tGlow);
        Memory.writeMemFrom<Chams>(ci.entity + pNetVars.m_clrRender, &clr_team);
    }
}

void Glow::ProcessEntityEnemy(const ClientInfo& ci, const Entity& e, uintptr_t& glowObject, bool targetindex) {
    if(glow_enabled){
        int glowIndex = Memory.readMem<int>(ci.entity + pNetVars.m_iGlowIndex);
        GlowObject eGlow;
        Memory.readMemTo<GlowObject>(glowObject + (glowIndex * 0x38), &eGlow);
        eGlow = setGlowColor(eGlow, e.health, ci.entity, targetindex);
        Memory.writeMemFrom<GlowObject>(glowObject + (glowIndex * 0x38), &eGlow);
        Memory.writeMemFrom<Chams>(ci.entity + pNetVars.m_clrRender, &clr_enemy);
    }
}

void Glow::ProcessTargetEntity(const ClientInfo& ci, uintptr_t& glowObject, ColorRGB &color) {
    if(glow_enabled){
        int glowIndex = Memory.readMem<int>(ci.entity + pNetVars.m_iGlowIndex);
        GlowObject targetGlow;
        Memory.readMemTo<GlowObject>(glowObject + (glowIndex * 0x38), &targetGlow);
        Functions::assignColorFromStruct(color, targetGlow);
        targetGlow.renderWhenOccluded = true;
        targetGlow.renderWhenUnoccluded = false;
        Memory.writeMemFrom<GlowObject>(glowObject + (glowIndex * 0x38), &targetGlow);
    }
}

void Glow::ProcessD3D9Render(const ClientInfo& ci, const Entity& e, int& index){
    if(master_esp_toggle){
        player_info player;
        //Functions::getPlayerInfo(index, player);
        view_matrix_t vm;
        Memory.readMemTo<view_matrix_t>(pOffsets.dwViewMatrix, &vm);    // read player viewmatrix

        VECTOR3 head = {head.x = e.vecOrigin.x, head.y = e.vecOrigin.y, head.z = e.vecOrigin.z + 75.f};
        VECTOR3 screenpos = Math::WorldToScreen(e.vecOrigin, &vm, Paint::width, Paint::height);
        VECTOR3 screenhead = Math::WorldToScreen(head, &vm, Paint::width, Paint::height);               // get player cooardinates
        int height_f = screenpos.y - screenhead.y;
        int width_f = height_f / 2;
        if(Paint::height < 1070 && Paint::width < 1900) // needed for smaller res, so boxes are drawn at correct position
            screenhead.y += 10;

        // = = = = = = = = = = = = = = = = = = =[   Draw on screen  ]= = = = = = = = = = = = = = = =

        if(screenpos.z >= 0.01f){
            if(boxes_enabled){
                if(boxes_by_health_enabled){
                    Paint::BorderBoxOutlined(screenhead.x - width_f / 2,screenhead.y,width_f,height_f,2,255,(int)(255 - (e.health * 2.55f)),(int)(e.health * 2.55f),0, 255, 0 ,1, 0); // 100 100 100 255
                } else {
                    Paint::BorderBoxOutlined(screenhead.x - width_f / 2,screenhead.y,width_f,height_f,2, espBoxColor);
                }
            }
            if(snapl_enabled)
                Paint::Line(Paint::width / 2,Paint::height - 1,screenhead.x, screenhead.y + height_f, snaplineColor, 1);    // draw snaplines

            if(weapon_health_enabled){
                int weaponId = Memory.readMem<int>(ci.entity + pNetVars.m_hActiveWeapon);
                int weaponEnt = Memory.readMem<int>(pOffsets.dwEntityList + ((weaponId & 0xFFF) - 1) * 0x10);
                if(weaponEnt != NULL){
                    int entityWeapon = Memory.readMem<int>(weaponEnt + pNetVars.m_iItemDefinitionIndex);
                    char str[64];
                    snprintf(str, 64, "%dhp", e.health);

                    Paint::StringOutlined((char*)player.name,screenhead.x - width_f / 2,screenhead.y - 15,255,0,1,0, 255, 255, 255 ,255);
                    Paint::StringOutlined((char*)Functions::getActiveWeapon(entityWeapon),screenhead.x - width_f / 2,screenhead.y + height_f + 5,255,0,1,0,255,255,255,255);
                    Paint::StringOutlined(str, screenhead.x - width_f / 2,screenhead.y + height_f + 15,255,0,1,0,255,255,255,255);
                }
                if(Functions::checkIfScoped(ci.entity))
                    Paint::StringOutlined((char*)"Scoped",screenhead.x - width_f / 2,screenhead.y - 30,255,0,1,0, 255, 65 ,255, 0);
            }
        }
    }
}
