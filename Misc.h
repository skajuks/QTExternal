#pragma once
#include "Structs.h"
#include "localplayer.h"

static const char* skybox_array[] = {"cs_baggage_skybox_",
                              "cs_tibet",
                              "vietnam",
                              "sky_lunacy",
                              "embassy",
                              "italy",
                              "jungle",
                              "office",
                              "sky_cs15_daylight01_hdr",
                              "sky_cs15_daylight02_hdr",
                              "nukeblank",
                              "sky_venice",
                              "sky_csgo_cloudy01",
                              "sky_csgo_night02",
                              "vertigo",
                              "vertigoblue_hdr",
                              "sky_dust",
                              "sky_hr_aztec"};

static const char* radio_commands[11] = {"getout", "enemydown", "enemyspot", "needbackup",
                                      "takingfire", "regroup", "sticktog", "go", "coverme",
                                      "cheer", "roger"};
static const std::vector<map> map_list = {
    {"cs_office", "maps/cs_office.png", -1838, 1858, 4.1f},
    {"de_mirage", "maps/de_mirage.png",0,0,0},
    {"de_dust2", "maps/de_dust2.png", -2476, 3239, 4.4f},
    {"de_inferno", "de_inferno.png",0,0,0},
    {"de_vertigo", "de_vertigo.png",0,0,0},
};

class Misc
{
public:
    static void setRadioSpamEnabled();
    static void fakeLagThreaded(const Entity& localPlayer, bool toggle);
    static bool returnAutoAcceptState();
    static void setAutoAcceptEnabled();
    static bool autoAccept();
    static void setBhop(const Entity& local_entity);
    static bool bhopEnabled();
    static void setBhopEnabled(bool state);
    static void setEnabledNoFlash(bool state);
    static void setNoFlash(const ClientInfo& localPlayer);
    static void setNoFlashAmount(float amount);
    static void setNightmodeAmount(const ClientInfo& localPlayer, float amount);
    static void changeFov(const ClientInfo& localPlayer, int fov);
    static void doBlockBot(float side, float forward);
    static void doorSpammer(int use);
    static void miscItemsThreaded(const Entity& localPlayer, const ClientInfo& ci);
    static void setDMExploitEnabled(bool state);
    static void setPerfectNadeEnabled();
};
