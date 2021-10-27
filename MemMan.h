#pragma once
#include <Windows.h>
#include <vector>
#include <TlHelp32.h>
#include <iostream>
#include "Structs.h"
#include "offset_fetch/offsets.hpp"

struct module{
    DWORD dwBase, dwSize;
};

struct Offsets{
    int cl_sidespeed;
    int cl_forwardspeed;
    int dwUse;
    UINT32 xor_cl_sidespeed;
    UINT32 xor_cl_forwardspeed;
    //
    uintptr_t dwForceJump;
    uintptr_t dwGlowObjectManager;
    uintptr_t dwEntityList;
    uintptr_t dwLocalPlayer;
    uintptr_t dwClientState;
    uintptr_t model_ambient_min;
    uintptr_t dwViewMatrix;
    uintptr_t dwbSendPackets;
    uintptr_t dwInput;
    uintptr_t clientstate_last_outgoing_command;
    uintptr_t dwClientState_PlayerInfo;
    uintptr_t dwClientState_ViewAngles;
    //
    uintptr_t offsetClientState;
    uintptr_t m_flNextCmdTime;
    uintptr_t g_bVoiceRecording;
    uintptr_t g_pClientClassHead;
    uintptr_t Callback__IsReady;
    uintptr_t ConfirmedReservationCallback;
    uintptr_t clientCmd_Unrestricted;
    uintptr_t modelInfoClient;
    uintptr_t loadNamedSkys;
    uintptr_t forward;
    uintptr_t back;
    uintptr_t right;
    uintptr_t left;
};

class MemMan
{
public:
	MemMan();
	~MemMan();
    DWORD pid;
    LPVOID alloc = 0;

    template <class val>
    val readMem(uintptr_t addr)
    {
        val x;
        ReadProcessMemory(handle, (LPBYTE*)addr, &x, sizeof(x), NULL);
        return x;
    }

    template <class val>
    constexpr void readMemTo(uintptr_t addr, val* readTo)
    {
        ReadProcessMemory(handle, (LPBYTE*)addr, readTo, sizeof(*readTo), NULL);
    }
    template <typename T>
    inline void read(uintptr_t address, T &value)
    {
        ReadProcessMemory(handle, reinterpret_cast<LPVOID>(address), &value, sizeof(T), NULL);
    }
    template <class val>
    val writeMem(uintptr_t addr, val x)
    {
        WriteProcessMemory(handle, (LPBYTE*)addr, &x, sizeof(x), NULL);
        return x;
    }

    template <typename T>
    inline void write(uintptr_t address, const T &value)
    {
        WriteProcessMemory(handle, reinterpret_cast<LPVOID>(address), &value, sizeof(T), 0);
    }

    template <class val>
    constexpr void writeMemFrom(uintptr_t addr, val* x)
    {
        WriteProcessMemory(handle, (LPBYTE*)addr, x, sizeof(*x), NULL);
    }

    HANDLE handle;

    void createThread(uintptr_t address, LPVOID param = 0);
    LPVOID getAlloc();
	uintptr_t getProcess(const wchar_t*);
	uintptr_t getModule(uintptr_t, const wchar_t*);
	uintptr_t getAddress(uintptr_t, std::vector<uintptr_t>);
    bool MemoryCompare(const BYTE* data, const BYTE* mask, const char* szMask);
    Offsets getOffsets(int id);
    int findPattern(byte pattern[], std::string mask, int moduleBase, int moduleSize, std::string name);
    MODULEENTRY32 getModuleInfo(const char* modName, DWORD proc_id);
    uintptr_t generateMask(std::string signature, int moduleBase, int moduleSize, std::string name);
private:


};

struct netVarStr{
    uintptr_t m_clrRender;
    uintptr_t m_iTeamNum;
    uintptr_t m_vecOrigin;
    uintptr_t m_nModelIndex;
    uintptr_t m_pBones;
    uintptr_t m_bGunGameImmunity;
    uintptr_t m_bHasDefuser;
    uintptr_t m_bIsDefusing;
    uintptr_t m_bIsScoped;
    uintptr_t m_bSpottedByMask;
    uintptr_t m_flCustomAutoExposureMax;
    uintptr_t m_flCustomAutoExposureMin;
    uintptr_t m_hActiveWeapon;
    uintptr_t m_iGlowIndex;
    uintptr_t m_iItemDefinitionIndex;
    uintptr_t m_flFlashDuration;
    uintptr_t m_dwBoneMatrix;
    uintptr_t m_aimPunchAngle;
    uintptr_t m_iDefaultFOV;
    uintptr_t m_hObserverTarget;
    uintptr_t m_bSpotted;
};

class NetVars : public MemMan
{
public:
    netVarStr dumpNetvars();
    RecvTable* getTable(const char* name);
    int getProp(RecvTable* pTable, const char* name);
    int getOffset(const char* table_name, const char* var_name);
};
