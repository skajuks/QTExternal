#pragma once
#include <Windows.h>
#include <vector>
#include <TlHelp32.h>
#include <iostream>

struct module{
    DWORD dwBase, dwSize;
};

struct Offsets{
    int cl_sidespeed;
    int cl_forwardspeed;
    int dwUse;
    UINT32 xor_cl_sidespeed;
    UINT32 xor_cl_forwardspeed;
    uintptr_t dwForceJump;
    uintptr_t dwGlowObjectManager;
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
    uintptr_t FindSignature(DWORD start, DWORD size, const char* sig ,const char* mask);
    bool MemoryCompare(const BYTE* data, const BYTE* mask, const char* szMask);
    Offsets getOffsets(int id);
    int findPattern(byte pattern[], std::string mask, int moduleBase, int moduleSize);
    MODULEENTRY32 getModuleInfo(const char* modName, DWORD proc_id);
    uintptr_t findPattern(byte pattern[], std::string mask, int moduleBase, int moduleSize, int offset);
private:


};
