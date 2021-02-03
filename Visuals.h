
#include <windows.h>


struct GlowObject {
    BYTE base[4];
    float red;
    float green;
    float blue;
    float alpha;
    BYTE buffer[16];
    bool renderWhenOccluded;
    bool renderWhenUnoccluded;
    bool fullBloom;
    BYTE buffer2[5];
    int glowstyle;
};

struct Chams {
    BYTE red;
    BYTE green;
    BYTE blue;
};

class Glow {
public:
    static bool glowEnabled();
    static void teamChamsStateChanged(int tChams[3]);
    static void enemyChamsStateChanged(int eChams[3]);
    static void brightnessStateChanged(bool state, float bright);
    static void setGlowAlpha(float value);
    static void setEnableHealthGlow(bool state);
    static void setGlowTeamColor(int r, int g, int b, bool lPlayerTeam);
    static void setGlowEnabled(bool state);
    static void ESP();
};

