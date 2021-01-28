#pragma once
#include <Windows.h>
#include <vector>

struct module{
    DWORD dwBase, dwSize;
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
	val writeMem(uintptr_t addr, val x)
	{
		WriteProcessMemory(handle, (LPBYTE*)addr, &x, sizeof(x), NULL);
		return x;
	}

	uintptr_t getProcess(const wchar_t*);
	uintptr_t getModule(uintptr_t, const wchar_t*);
	uintptr_t getAddress(uintptr_t, std::vector<uintptr_t>);
    //uintptr_t FindSignature(char* mod, const char* sig ,const char* mask);
    //bool MemoryCompare(const BYTE* data, const BYTE* mask, const char* szMask);
	
private:
    //module getModuleInfo(char* moduleName);
    HANDLE handle;
};
