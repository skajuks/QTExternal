#include "MemMan.h"
#include <TlHelp32.h>
#include <iostream>
#include <iomanip>
#include "Functions.h"

MemMan::MemMan()
{
	handle = NULL;
}

MemMan::~MemMan()
{
	CloseHandle(handle);
}


uintptr_t MemMan::getProcess(const wchar_t* proc)
{
	HANDLE hProcessId = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	uintptr_t process;
	PROCESSENTRY32 pEntry;
	pEntry.dwSize = sizeof(pEntry);

	do
	{
		if (!_wcsicmp(pEntry.szExeFile, proc))
		{
			process = pEntry.th32ProcessID;
			CloseHandle(hProcessId);
			handle = OpenProcess(PROCESS_ALL_ACCESS, false, process);
		}

	} while (Process32Next(hProcessId, &pEntry));
    pid = process;
	return process;
}

uintptr_t MemMan::getModule(uintptr_t procId, const wchar_t* modName)
{
	HANDLE hModule = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, procId);
	MODULEENTRY32 mEntry;
	mEntry.dwSize = sizeof(mEntry);

	do
	{
		if (!_wcsicmp(mEntry.szModule, modName))
		{
			CloseHandle(hModule);
            return (uintptr_t)mEntry.modBaseAddr;
		}
	} while (Module32Next(hModule, &mEntry));
	return 0;
}

uintptr_t MemMan::getAddress(uintptr_t addr, std::vector<uintptr_t> vect)
{
	for (int i = 0; i < vect.size(); i++)
	{
		ReadProcessMemory(handle, (BYTE*)addr, &addr, sizeof(addr), 0);
		addr += vect[i];
	}
	return addr;
}


bool MemMan::MemoryCompare(const BYTE* data, const BYTE* mask, const char* szMask)
{
    for(; *szMask; ++szMask, ++data, ++mask){
        if(*szMask == 'x' && *data != *mask)
            return false;
    }
    return(*szMask == NULL);
}

MODULEENTRY32 MemMan::getModuleInfo(const char* modName, DWORD pid)
{
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);
        if (hSnap != INVALID_HANDLE_VALUE) {
            MODULEENTRY32 modEntry;
            modEntry.dwSize = sizeof(modEntry);
            if (Module32First(hSnap, &modEntry)) {
                do {
                    if (!_wcsicmp(modEntry.szModule, (const wchar_t*)modName)) {
                        CloseHandle(hSnap);
                        return modEntry;
                    }
                } while (Module32Next(hSnap, &modEntry));
            }
        }
        MODULEENTRY32 module = {};
        return module;
}


uintptr_t MemMan::findPattern(MODULEENTRY32 client, uint8_t* arr, const char* pattern, int offset, int extra) {
    uintptr_t scan = 0x0;
    const char* pat = pattern;
    uintptr_t firstMatch = 0;
    for (uintptr_t pCur = (uintptr_t)arr; pCur < (uintptr_t)arr + client.modBaseSize; ++pCur) {
        if (!*pat) { scan = firstMatch; break; }
        if (*(uint8_t*)pat == '\?' || *(uint8_t*)pCur == ((((pat[0] & (~0x20)) >= 'A' && (pat[0] & (~0x20)) <= 'F') ? ((pat[0] & (~0x20)) - 'A' + 0xa) : ((pat[0] >= '0' && pat[0] <= '9') ? pat[0] - '0' : 0)) << 4 | (((pat[1] & (~0x20)) >= 'A' && (pat[1] & (~0x20)) <= 'F') ? ((pat[1] & (~0x20)) - 'A' + 0xa) : ((pat[1] >= '0' && pat[1] <= '9') ? pat[1] - '0' : 0)))) {
            if (!firstMatch) firstMatch = pCur;
            if (!pat[2]) { scan = firstMatch; break; }
            if (*(WORD*)pat == 16191 /*??*/ || *(uint8_t*)pat != '\?') pat += 3;
            else pat += 2;
        }
        else { pat = pattern; firstMatch = 0; }
    }
    if (!scan) return 0x0;
    uint32_t read;
    ReadProcessMemory(handle, (void*)(scan - (uintptr_t)arr + (uintptr_t)client.modBaseAddr + offset), &read, sizeof(read), NULL);
    return read + extra;
}

uintptr_t MemMan::FindSignature(DWORD start, DWORD size, const char* sig ,const char* mask)
{

    BYTE* data = new BYTE[size];
    SIZE_T bytesRead;

    ReadProcessMemory(handle, (LPVOID)start, data, size, &bytesRead);

    for(uintptr_t i = 0; i< size; i++)
    {
        if(MemoryCompare((const BYTE*)(data + i),(const BYTE*)sig, mask)){
            delete[] data;
            return start + i;
        }
    }
    delete[] data;
    return NULL;
}

Offsets MemMan::getOffsets(int id){
    Offsets pOffsets;
    HANDLE h = OpenProcess(PROCESS_ALL_ACCESS, false ,id);
    MODULEENTRY32 client = getModuleInfo("client.dll", id);
    auto bytes = new uint8_t[client.modBaseSize];
    DWORD bytes_read;
    ReadProcessMemory(h, client.modBaseAddr, bytes, client.modBaseSize, &bytes_read);
    if(bytes_read != client.modBaseSize) throw;

    pOffsets.dwLocalPlayer = findPattern(client, bytes,
                                         "8D 34 85 ? ? ? ? 89 15 ? ? ? ? 8B 41 08 8B 48 04 83 F9 FF",
                                         3, 4);
    //pOffsets.dwLocalPlayer = pOffsets.dwLocalPlayer - (uintptr_t)client.modBaseAddr;

    pOffsets.dwEntityList = findPattern(client, bytes,
                                        "BB ? ? ? ? 83 FF 01 0F 8C ? ? ? ? 3B F8",
                                        1, 0);
    //pOffsets.dwEntityList = pOffsets.dwEntityList - (uintptr_t)client.modBaseAddr;

    delete[] bytes;

    /*pOffsets.dwLocalPlayer = FindSignature(clientDLL.dwBase, clientDLL.dwSize,
                                           "\x8D\x34\x85\x00\x00\x00\x00\x89\x15\x00\x00\x00\x00\x8B\x41\x08\x8B\x48\x04\x83\xF9\xFF",
                                           "xxx????xx????xxxxxxxxx");
    pOffsets.dwLocalPlayer = readMem<uintptr_t>(pOffsets.dwLocalPlayer + 4);
    pOffsets.dwLocalPlayer = readMem<uintptr_t>(pOffsets.dwLocalPlayer);*/
    //pOffsets.dwLocalPlayer = pOffsets.dwLocalPlayer - gameModule;
    return pOffsets;
}
