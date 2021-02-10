#pragma once
#include <Windows.h>
#include <vector>
#include <TlHelp32.h>

struct module{
    DWORD dwBase, dwSize;
};

struct Offsets{
    uintptr_t dwLocalPlayer;
    uintptr_t dwGlowObjectManager;
    uintptr_t dwEntityList;

};

class MemMan
{
public:
	MemMan();
	~MemMan();
    DWORD pid;

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

    template <class val>
    constexpr void writeMemFrom(uintptr_t addr, val* x)
    {
        WriteProcessMemory(handle, (LPBYTE*)addr, x, sizeof(*x), NULL);
    }

	uintptr_t getProcess(const wchar_t*);
	uintptr_t getModule(uintptr_t, const wchar_t*);
	uintptr_t getAddress(uintptr_t, std::vector<uintptr_t>);
    uintptr_t FindSignature(DWORD start, DWORD size, const char* sig ,const char* mask);
    bool MemoryCompare(const BYTE* data, const BYTE* mask, const char* szMask);
    Offsets getOffsets(int id);
    uintptr_t findPattern(MODULEENTRY32 client, uint8_t* arr, const char* pattern, int offset, int extra);
    MODULEENTRY32 getModuleInfo(const char* modName, DWORD proc_id);
private:
    HANDLE handle;

};
