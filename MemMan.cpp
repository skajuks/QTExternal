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

module targetModule;

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

#define SIZE_CLIENTCMD (15 + 128)
#define SIZE_LOADSKY (13 + 48)

const size_t allocSize = 495;

LPVOID MemMan::getAlloc(){
    if(!alloc){
        alloc = VirtualAllocEx(handle, 0, allocSize, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
        if(!alloc)
           std::cout << "Warning: VirtualAllocEx failed" << std::endl;
    }
    return alloc;
}

void MemMan::createThread(uintptr_t address, LPVOID param){
    HANDLE hThread = CreateRemoteThread(handle, 0, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(address), param, 0, 0);
    if(!hThread){
        std::cout << "CreateRemoteThread failed!" << std::endl;
        return;
    }
    WaitForSingleObject(hThread, 5000);
    CloseHandle(hThread);
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

int MemMan::findPattern(byte pattern[], std::string mask, int moduleBase, int moduleSize)
{
    BYTE* moduleBytes = new BYTE[moduleSize];
    SIZE_T numBytes = 0;

    if (ReadProcessMemory(handle, (LPCVOID)moduleBase, moduleBytes, (uintptr_t)moduleSize, &numBytes))
    {
        for (int i = 0; i < moduleSize; i++)
        {
            bool found = true;

            for (size_t l = 0; l < mask.length(); l++)
            {
                found = mask[l] == '?' || moduleBytes[l + i] == pattern[l];

                if (!found)
                    break;
            }

            if (found)
                return i;
        }
    }
    std::cout << "not found" << std::endl;
    return 0;
}

uintptr_t MemMan::findPattern(byte pattern[], std::string mask, int moduleBase, int moduleSize, int offset)
{
    BYTE* moduleBytes = new BYTE[moduleSize];
    SIZE_T numBytes = 0;

    if (ReadProcessMemory(handle, (LPCVOID)moduleBase, moduleBytes, (uintptr_t)moduleSize, &numBytes))
    {
        for (int i = 0; i < moduleSize; i++)
        {
            bool found = true;

            for (size_t l = 0; l < mask.length(); l++)
            {
                found = mask[l] == '?' || moduleBytes[l + i] == pattern[l];

                if (!found)
                    break;
            }

            if (found)
                return i + offset;
        }
    }
    std::cout << "not found" << std::endl;
    return 0;
}


module GetModule(const wchar_t* moduleName, int id) {
    HANDLE hmodule = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, id);
    MODULEENTRY32 mEntry;
    mEntry.dwSize = sizeof(mEntry);

    do {
        if (!_wcsicmp(mEntry.szModule, moduleName)) {//_tcscmp
            CloseHandle(hmodule);

            targetModule = { (DWORD)mEntry.hModule, mEntry.modBaseSize };
            return targetModule;
        }
    } while (Module32Next(hmodule, &mEntry));

    module mod = { (DWORD64)false, (DWORD64)false };
    return mod;
}

UINT32 calcXorWithValue(int cvarOffset)
        {
            return Memory.readMem<UINT32>(gameModule + cvarOffset) - 0x2C;
        }


Offsets MemMan::getOffsets(int id){
    Offsets pOffsets;
    module enginemod = GetModule(L"engine.dll", id);
    module mod = GetModule(L"client.dll", id);
    pOffsets.dwForceJump = findPattern(new byte[] { 0x8B, 0x0D, 0x00, 0x00, 0x00, 0x00, 0x8B, 0xD6, 0x8B, 0xC1, 0x83, 0xCA, 0x02}, "xx????xxxxxxx", mod.dwBase, mod.dwSize, 0x02);
    pOffsets.cl_sidespeed = findPattern(new byte[] { 0xF3, 0x0F, 0x10, 0x05, 0x00, 0x00, 0x00, 0x00, 0xF3, 0x0F, 0x11, 0x44, 0x24, 0x00, 0x81, 0x74, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD9, 0x44, 0x24, 0x14, 0xEB, 0x07 }, "xxxx????xxxxx?xxx?????xxxxxx", mod.dwBase, mod.dwSize) + 0x4;
    pOffsets.cl_forwardspeed = findPattern(new byte[] { 0xF3, 0x0F, 0x10, 0x05, 0x00, 0x00, 0x00, 0x00, 0xF3, 0x0F, 0x11, 0x44, 0x24, 0x00, 0x81, 0x74, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0xEB, 0x37 }, "xxxx????xxxxx?xxx?????xx", mod.dwBase, mod.dwSize) + 0x4;
    pOffsets.dwUse = findPattern(new byte[] { 0x8B, 0x0D, 0x00, 0x00, 0x00, 0x00, 0x8B, 0xF2, 0x8B, 0xC1, 0x83, 0xCE, 0x20 }, "xx????xxxxxxx", mod.dwBase, mod.dwSize) + 2;
    pOffsets.clientCmd_Unrestricted = findPattern(new byte[] { 0x55,0x8B,0xEC,0x8B,0x0D,0x00,0x00,0x00,0x00,0x81,0xF9,0x00,0x00,0x00,0x00,0x75,0x0C,0xA1,0x00,0x00,0x00,0x00,0x35,0x00,0x00,0x00,0x00,0xEB,0x05,0x8B,0x01,0xFF,0x50,0x34,0x50},
    "xxxxx????xx????xxx????x????xxxxxxxx", enginemod.dwBase, enginemod.dwSize);
    //pOffsets.modelInfoClient = findPattern(new byte[] { 0x8D,0x44,0x24,0x3C,0x8B,0xF1,0x50,0x8D,0x44,0x24,0x3C,0xB9}, "xxxxxxxxxxxx", enginemod.dwBase, enginemod.dwSize) + 0xC;
    //pOffsets.loadNamedSkys = findPattern(new byte[] { 0x55,0x8B,0xEC,0x81,0xEC,0x34,0x01,0x00,0x00,0x56}, "xxxxxxxxxx", enginemod.dwBase, enginemod.dwSize);
    pOffsets.loadNamedSkys = findPattern(new byte[] { 0x55,0x8B,0xEC,0x81,0xEC,0x00,0x00,0x00,0x00,0x56, 0x57, 0x8B, 0xF9, 0xC7, 0x45}, "xxxxx????xxxxxx", enginemod.dwBase, enginemod.dwSize);

    pOffsets.xor_cl_sidespeed = calcXorWithValue(pOffsets.cl_sidespeed);
    pOffsets.xor_cl_forwardspeed = calcXorWithValue(pOffsets.cl_forwardspeed);



    std::cout << std::hex << pOffsets.loadNamedSkys << std::endl;
    return pOffsets;
}
