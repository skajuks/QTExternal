
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
    static void setGlowAlpha(float value);
    static void setEnableHealthGlow(bool state);
    static void setGlowTeamColor(int r, int g, int b, bool lPlayerTeam);
    static bool glowEnabled();
    static void setGlowEnabled(bool state);
    static void ESP();
};

