#include "MemMan.h"
#include <iostream>
#include <iomanip>
#include "Functions.h"
#include <sstream>
#include <vector>

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

int MemMan::findPattern(byte pattern[], std::string mask, int moduleBase, int moduleSize, std::string name)
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

    std::cout << "Failed to dump offset " + name << std::endl;
    return 0;
}

uintptr_t MemMan::generateMask(std::string signature, int moduleBase, int moduleSize, std::string name){
    std::vector<byte> temp;
    std::istringstream iss(signature);
    std::string s;
    while(std::getline(iss, s, ' ')){
       if(s == "?"){
           temp.push_back(0);
       } else {
           temp.push_back(strtoul(s.c_str(), nullptr, 16));
       }
    }

    std::string mask;
    for(auto b : temp) {
        mask += b ? "x" : "?";
    }
    return findPattern(temp.data(), mask, moduleBase, moduleSize, name);
}

UINT32 calcXorWithValue(int cvarOffset)
{
    return Memory.readMem<UINT32>(gameModule + cvarOffset) - 0x2C;
}



/* Offsets and netwars */

Offsets MemMan::getOffsets(int id){
    Offsets pOffsets;

    /* get module base addresses and sizes */

    module enginemod = GetModule(L"engine.dll", id);
    module mod = GetModule(L"client.dll", id);

    // special offsets that cannot be found on hazeddumper

    //pOffsets.offsetClientState = Memory.readMem<uintptr_t>(engineModule + generateMask("8B 3D ? ? ? ? 8A F9 F3 0F 11 45 FC", enginemod.dwBase, enginemod.dwSize) + 2);

    std::cout << "Dumping offsets.." << std::endl;
    pOffsets.m_flNextCmdTime = generateMask("F2 0F 10 87 ? ? ? ? 66 0F 2F 05", enginemod.dwBase, enginemod.dwSize, "m_flNextCmdTime") + 4 + pOffsets.offsetClientState;
    //pOffsets.g_bVoiceRecording = findPattern(new byte[] { 0x80, 0x3D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x75, 0x0C, 0x6A }, "xx????xxxx", enginemod.dwBase, enginemod.dwSize, "g_bVoiceRecording") + 2;
    //pOffsets.cl_sidespeed = findPattern(new byte[] { 0xF3, 0x0F, 0x10, 0x05, 0x00, 0x00, 0x00, 0x00, 0xF3, 0x0F, 0x11, 0x44, 0x24, 0x00, 0x81, 0x74, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD9, 0x44, 0x24, 0x14, 0xEB, 0x07 }, "xxxx????xxxxx?xxx?????xxxxxx", mod.dwBase, mod.dwSize, "cl_sidespeed") + 0x4;
    //pOffsets.cl_forwardspeed = findPattern(new byte[] { 0xF3, 0x0F, 0x10, 0x05, 0x00, 0x00, 0x00, 0x00, 0xF3, 0x0F, 0x11, 0x44, 0x24, 0x00, 0x81, 0x74, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0xEB, 0x37 }, "xxxx????xxxxx?xxx?????xx", mod.dwBase, mod.dwSize, "cl_forwardspeed") + 0x4;
    //pOffsets.dwUse = findPattern(new byte[] { 0x8B, 0x0D, 0x00, 0x00, 0x00, 0x00, 0x8B, 0xF2, 0x8B, 0xC1, 0x83, 0xCE, 0x20 }, "xx????xxxxxxx", mod.dwBase, mod.dwSize, "dwUse") + 2;
    pOffsets.clientCmd_Unrestricted = generateMask("55 8B EC 8B 0D ? ? ? ? 81 F9 ? ? ? ? 75 0C A1 ? ? ? ? 35 ? ? ? ? EB 05 8B 01 FF 50 34 50", enginemod.dwBase, enginemod.dwSize, "clientCmd_Unrestricted");
    //pOffsets.loadNamedSkys = findPattern(new byte[] { 0x55,0x8B,0xEC,0x81,0xEC,0x00,0x00,0x00,0x00,0x56, 0x57, 0x8B, 0xF9, 0xC7, 0x45}, "xxxxx????xxxxxx", enginemod.dwBase, enginemod.dwSize, "loadNamedSkys");
    pOffsets.forward = generateMask("55 8B EC 51 53 8A 5D 08", mod.dwBase, mod.dwSize, "forward");
    pOffsets.back = Memory.readMem<uintptr_t>(gameModule + pOffsets.forward + 0x11F);
    pOffsets.left = Memory.readMem<uintptr_t>(gameModule + pOffsets.forward + 0x1D1);
    pOffsets.right = Memory.readMem<uintptr_t>(gameModule + pOffsets.forward + 0x200);
    pOffsets.forward = Memory.readMem<uintptr_t>(gameModule + pOffsets.forward + 0xF5);
    pOffsets.Callback__IsReady = Memory.readMem<uintptr_t>(gameModule + generateMask("56 8B 35 ? ? ? ? 57 83 BE", mod.dwBase, mod.dwSize, "Callback__IsReady")) - 4;
    pOffsets.ConfirmedReservationCallback = Memory.readMem<uintptr_t>(pOffsets.Callback__IsReady + 7);

    /* general dump of offsets */

    pOffsets.g_pClientClassHead = Memory.readMem<uintptr_t>(gameModule + generateMask("A1 ? ? ? ? C3 CC CC CC CC CC CC CC CC CC CC A1 ? ? ? ? B9", mod.dwBase, mod.dwSize, "g_pClientClassHead") + 1);
    pOffsets.dwForceJump = Memory.readMem<uint32_t>(gameModule + generateMask("8B 0D ? ? ? ? 8B D6 8B C1 83 CA 02", mod.dwBase, mod.dwSize, "dwForceJump") + 2) + 0;
    pOffsets.dwGlowObjectManager = Memory.readMem<uint32_t>(gameModule + generateMask("A1 ? ? ? ? A8 01 75 4B", mod.dwBase, mod.dwSize, "dwGlowObjectManager") + 1) + 4;
    pOffsets.dwEntityList = Memory.readMem<uint32_t>(gameModule + generateMask("BB ? ? ? ? 83 FF 01 0F 8C ? ? ? ? 3B F8", mod.dwBase, mod.dwSize, "dwEntityList") + 1) + 0;
    pOffsets.dwLocalPlayer = Memory.readMem<uint32_t>(gameModule + generateMask("8D 34 85 ? ? ? ? 89 15 ? ? ? ? 8B 41 08 8B 48 04 83 F9 FF", mod.dwBase, mod.dwSize, "dwLocalPlayer") + 3) + 4;
    pOffsets.dwClientState = Memory.readMem<uint32_t>(engineModule + generateMask("A1 ? ? ? ? 33 D2 6A 00 6A 00 33 C9 89 B0", enginemod.dwBase, enginemod.dwSize, "dwClientState") + 1) - engineModule;
    pOffsets.model_ambient_min = Memory.readMem<uint32_t>(engineModule + generateMask("F3 0F 10 0D ? ? ? ? F3 0F 11 4C 24 ? 8B 44 24 20 35 ? ? ? ? 89 44 24 0C", enginemod.dwBase, enginemod.dwSize, "model_ambient_min") + 4);
    pOffsets.dwViewMatrix = Memory.readMem<uint32_t>(gameModule + generateMask("0F 10 05 ? ? ? ? 8D 85 ? ? ? ? B9", mod.dwBase, mod.dwSize, "dwViewMatrix") + 3) + 176;
    pOffsets.dwbSendPackets = Memory.readMem<uintptr_t>(engineModule + generateMask("B3 01 8B 01 8B 40 10 FF D0 84 C0 74 0F 80 BF ? ? ? ? ? 0F 84", enginemod.dwBase, enginemod.dwSize, "dwbSendPackets")) + 1;
    pOffsets.dwInput = Memory.readMem<uint32_t>(gameModule + generateMask("B9 ? ? ? ? F3 0F 11 04 24 FF 50 10", mod.dwBase, mod.dwSize, "dwInput") + 1);
    pOffsets.clientstate_last_outgoing_command = Memory.readMem<uint32_t>(engineModule + generateMask("8B 8F ? ? ? ? 8B 87 ? ? ? ? 41", enginemod.dwBase, enginemod.dwSize, "clientstate_last_outgoing_command") + 2);
    pOffsets.dwClientState_PlayerInfo = Memory.readMem<uint32_t>(engineModule + generateMask("8B 89 ? ? ? ? 85 C9 0F 84 ? ? ? ? 8B 01", enginemod.dwBase, enginemod.dwSize, "dwClientState_PlayerInfo") + 2);
    pOffsets.dwClientState_ViewAngles = Memory.readMem<uint32_t>(engineModule + generateMask("F3 0F 11 86 ? ? ? ? F3 0F 10 44 24 ? F3 0F 11 86", enginemod.dwBase, enginemod.dwSize, "dwClientState_ViewAngles") + 4);

    // xored offsets

    pOffsets.xor_cl_sidespeed = calcXorWithValue(pOffsets.cl_sidespeed);
    pOffsets.xor_cl_forwardspeed = calcXorWithValue(pOffsets.cl_forwardspeed);

    std::cout << pOffsets.g_pClientClassHead << " " << pyfetcher::dwGetAllClasses << std::endl;

    int size = (int)(sizeof(pOffsets) / sizeof(uintptr_t));
    std::cout << "Offsets dump done! found " + std::to_string(size) + " offsets" << std::endl;
    return pOffsets;
}

///////////////////////////////////////////////////////////////
/// Netvars
///////////////////////////////////////////////////////////////

RecvTable* NetVars::getTable(const char* name){
    ClientClass cc;
    Memory.read<ClientClass>(gameModule + pyfetcher::dwGetAllClasses, cc);

    while(1 < 2){
        char varname[64];
        Memory.read<char[64]>(reinterpret_cast<uintptr_t>(cc.m_pNetworkName), varname);
        if(!strcmp(varname, name))
            return cc.m_pRecvTable;
        if(cc.m_pNext == 0)
            break;
        Memory.read<ClientClass>(reinterpret_cast<uintptr_t>(cc.m_pNext), cc);
    }
    return nullptr;
}

int NetVars::getProp(RecvTable* pTable, const char* name){
    RecvTable recvt;
    Memory.read<RecvTable>(reinterpret_cast<uintptr_t>(pTable), recvt);

    int offset = 0;
    for(int i = 0; i < recvt.m_nProps; i++){
        RecvProp prop;
        Memory.read<RecvProp>(reinterpret_cast<uintptr_t>(recvt.m_pProps) + i * sizeof (RecvProp), prop);
        if(prop.m_RecvType == DPT_DataTable){
            int extra = getProp(prop.m_pDataTable, name);
            if(extra)
                offset += prop.m_Offset + extra;
        } else {
            char varname[64];
            Memory.read<char[64]>(reinterpret_cast<uintptr_t>(prop.m_pVarName), varname);

            if(!strcmp(varname, name))
                return prop.m_Offset;
        }
    }
    return offset;
}

int NetVars::getOffset(const char* table_name, const char* var_name){
    RecvTable* recvt = getTable(table_name);
    std::string f = "Failed to find table ";
    std::string comb = f + var_name;
    if(!recvt){
        std::cout << comb << std::endl;

        return 0;
    }
    int offset = getProp(recvt, var_name);
    if(!offset){
        std::cout << "Failed to find offset" << std::endl;
        return 0;
    }

    std::cout << "Found " << var_name << "-> 0x" << std::hex << offset << std::endl;
    return offset;
}

netVarStr NetVars::dumpNetvars(){
    netVarStr pNetVars;
    printf("Dumping netvars..\n");
    pNetVars.m_bSpottedByMask = getOffset("CBaseEntity", "m_bSpottedByMask");
    pNetVars.m_iTeamNum = getOffset("CBasePlayer", "m_iTeamNum");
    pNetVars.m_clrRender = getOffset("CBaseEntity", "m_clrRender");
    pNetVars.m_bGunGameImmunity = getOffset("CCSPlayer", "m_bGunGameImmunity");
    pNetVars.m_bHasDefuser = getOffset("CCSPlayer", "m_bHasDefuser");
    pNetVars.m_bIsScoped = getOffset("CCSPlayer", "m_bIsScoped");
    pNetVars.m_bIsDefusing = getOffset("CCSPlayer", "m_bIsDefusing");
    pNetVars.m_iGlowIndex = getOffset("CCSPlayer", "m_flFlashDuration") + 24;
    pNetVars.m_hActiveWeapon = getOffset("CBasePlayer", "m_hActiveWeapon");
    pNetVars.m_iItemDefinitionIndex = getOffset("CBaseCombatWeapon", "m_iItemDefinitionIndex");
    pNetVars.m_flFlashDuration = pNetVars.m_iGlowIndex - 24;
    pNetVars.m_dwBoneMatrix = getOffset("CBaseAnimating", "m_nForceBone") + 28;
    pNetVars.m_aimPunchAngle = getOffset("CBasePlayer", "m_aimPunchAngle");
    pNetVars.m_iDefaultFOV = getOffset("CCSPlayer", "m_iDefaultFOV");
    pNetVars.m_hObserverTarget = getOffset("CBasePlayer", "m_hObserverTarget");
    pNetVars.m_bSpotted = getOffset("CBaseEntity", "m_bSpotted");
    return pNetVars;
}
