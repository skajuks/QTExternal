#ifndef STRUCTS_H
#define STRUCTS_H
#include <windows.h>
#include "csmath.h"
#include <stdint.h>

struct toggleStateData {
    const char* text;
    bool state;
};

class player_info {
    private:
        char __pad1[0x8];
    public:
        __int64 steamID64;
    //private:
        //char __pad[0x4];
    public:
        char name[32];
    private:
        char __pad2[0x64];
    public:
        char steam_id[32];
};

struct Color3 {
    float r;
    float g;
    float b;
};

enum SendPropType
{
    DPT_Int = 0,
    DPT_Float,
    DPT_Vector,
    DPT_VectorXY,	// Only encodes the XY of a vector, ignores Z
    DPT_String,
    DPT_Array,		// An array of the base types (can't be of datatables).
    DPT_DataTable,
    DPT_Int64,
    DPT_NUMSendPropTypes
};

struct RecvTable; // forward define for recvprop

struct RecvProp
{
    char*               m_pVarName;
    SendPropType        m_RecvType;
    int                 m_Flags;
    int                 m_StringBufferSize;
    bool                m_bInsideArray;
    const void*         m_pExtraData;
    RecvProp*           m_pArrayProp;
    void*               m_ArrayLengthProxy;
    void*               m_ProxyFn;
    void*               m_DataTableProxyFn;
    RecvTable*          m_pDataTable;
    int                 m_Offset;
    int                 m_ElementStride;
    int                 m_nElements;

    // If it's one of the numbered "000", "001", etc properties in an array, then
    // these can be used to get it's array property name for debugging.
    const char*         m_pParentArrayPropName;
};

struct RecvTable
{
    RecvProp*   m_pProps;
    int			m_nProps;
    void*       m_pDecoder;
    char*       m_pNetTableName;	// The name matched between client and server.
    bool		m_bInitialized;
    bool		m_bInMainList;
};

struct ClientClass
{
    void*           m_pCreateFn;
    void*           m_pCreateEventFn;
    char*           m_pNetworkName;
    RecvTable*      m_pRecvTable;
    ClientClass*    m_pNext;
    int             m_ClassID;
};

struct IBaseClientDLL
{
    virtual void fn0() = 0;
    virtual void fn1() = 0;
    virtual void fn2() = 0;
    virtual void fn3() = 0;
    virtual void fn4() = 0;
    virtual void fn5() = 0;
    virtual void fn6() = 0;
    virtual void fn7() = 0;

    virtual ClientClass* getAllClasses() = 0;
};

struct aimbotVariables {
    int entityWeapon;
    VECTOR3 recoil;
    VECTOR3 AimAngle;
};

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
    BYTE _filler0[8];
    float red;
    float green;
    float blue;
    float alpha;
    BYTE _filler[16];
    bool renderWhenOccluded;
    bool renderWhenUnoccluded;
};

/*struct GlowObjectDefinition_t
{
  int m_nNextFreeSlot;
  C_BaseEntity* m_pEntity;
  Vector m_vGlowColor;
  float m_flGlowAlpha;
  bool m_bGlowAlphaCappedByRenderAlpha;
  float m_flGlowAlphaFunctionOfMaxVelocity;
  float m_flGlowAlphaMax;
  float m_flGlowPulseOverdrive;
  bool m_bRenderWhenOccluded;
  bool m_bRenderWhenUnoccluded;
  bool m_bFullBloomRender;
  int m_nFullBloomStencilTestValue;
  int m_nRenderStyle;
  int m_nSplitScreenSlot;
};*/

struct Chams {
    BYTE red;
    BYTE green;
    BYTE blue;
};

struct Entity {
    BYTE pad_0000[112]; //0x0000

    int32_t clrRender; // 4bytes 0x0070

    BYTE pad_00001[128];

    int32_t team;   // 4 bytes 0x0F4

    BYTE pad_0001[8];

    int32_t health; // 4 bytes 0x0100
    int32_t flags; // 4 bytes 0x0104   
    VECTOR3 vecViewOffset; // 12 bytes 0x0108
    VECTOR3 vecVelocity; // 12 bytes 0x0114

    BYTE pad_0003[22];

    VECTOR3 vecOrigin;  // 12 bytes 0x0138

}; //Size: 0x0104

struct ClientInfo {
    uintptr_t entity; //0x0000
    BYTE _filler[8];
    uintptr_t nextEntity; //0x000C
}; //Size: 0x0010


#endif // STRUCTS_H
