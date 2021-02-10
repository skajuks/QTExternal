#ifndef STRUCTS_H
#define STRUCTS_H
#include <windows.h>
#include "csmath.h"
#include <stdint.h>


struct CInput {
    BYTE __pad0x01[0xF1]; //we dont need these vars
    DWORD Commands;
    DWORD VerifiedCommands;
};

struct CUserCmd {
    DWORD Vft;
    DWORD CmdNumber;
    DWORD TickCount;
    VECTOR3 ViewAngles;
    VECTOR3 AimDirection;
    FLOAT Forwardmove;
    FLOAT Sidemove;
    FLOAT Upmove;
    DWORD Buttons;
    BYTE Impulse;
    BYTE __pad0x01[0x03];
    DWORD WeaponSelect;
    DWORD WeaponSubtype;
    DWORD RandomSeed;
    WORD MouseDx;
    WORD MouseDy;
    BOOLEAN HasBeenPredicted;
    BYTE __pad0x02[0x1B];
};

struct CVerifiedUserCmd {
    CUserCmd Command;
    DWORD CRC;
};

struct GlowObject {
    BYTE _filler0[4];
    float red;
    float green;
    float blue;
    float alpha;
    BYTE _filler[16];
    bool renderWhenOccluded;
    bool renderWhenUnoccluded;
    bool fullBloom;
    BYTE _filler2[5];
    int glowstyle;
};

struct Chams {
    BYTE red;
    BYTE green;
    BYTE blue;
};

struct Entity {
    BYTE pad_0000[256]; //0x0000
    int32_t health; //0x0100
    int32_t flags; //0x0104
}; //Size: 0x0104

struct ClientInfo {
    uintptr_t entity; //0x0000
    BYTE _filler[8];
    uintptr_t nextEntity; //0x000C
}; //Size: 0x0010


#endif // STRUCTS_H
