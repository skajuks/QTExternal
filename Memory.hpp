#pragma once
#include <windows.h>
#include <iostream>
#include <vector>
#include <thread>
#include <TlHelp32.h>
#include <chrono>
#ifndef NCURL
//#include <curl/curl.h>
#endif // !NCURL


extern const size_t g_AllocSize;


struct CURLCallback
{
    char*	data;
    size_t	size;

    CURLCallback()
    {
        data = (char*)malloc(1);
        size = 0;
    }

    ~CURLCallback()
    {
        free(data);
    }

    CURLCallback(const CURLCallback&) = delete;
};

static size_t CURLWriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
    CURLCallback* mem = (CURLCallback*)userp;
    mem->data = (char*)realloc(mem->data, mem->size + nmemb + 1);
    if (mem->data == nullptr)
    {
        std::cerr << "Error: Not enough memory (realloc returned NULL)\n";
        return 0;
    }

    memcpy(&(mem->data[mem->size]), contents, nmemb);
    mem->size += nmemb;
    mem->data[mem->size] = '\0';

    return nmemb;
}

namespace Mem
{
    static DWORD	PID;
    static HANDLE	Process;
    static LPVOID	Alloc = 0;
    static char	Dir[MAX_PATH] = { 0 };

    struct CModCache
    {
        MODULEENTRY32	Entry;
        uint8_t*		Content;
    };
    static std::vector<CModCache> ModulesCache;



    static void Attach()
    {
        HWND hwnd = FindWindowA("Valve001", 0);

        while (!hwnd)
        {
            std::this_thread::sleep_for(std::chrono::seconds(30));
            hwnd = FindWindowA("Valve001", 0);
        }

        GetWindowThreadProcessId(hwnd, &PID);
        Process = OpenProcess((PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE), FALSE, PID);
        //if (!Process)
            //ErrorExit("Failed to open a process");
    }

    static void Detach()
    {
        VirtualFreeEx(Process, Alloc, 0, MEM_RELEASE);
        CloseHandle(Process);
    }

    static MODULEENTRY32* GetModuleEntry(const char* szMod)
    {
        for (size_t i = 0; i < ModulesCache.size(); ++i)
        {
            if (!strcmp((const char*)ModulesCache[i].Entry.szModule, szMod))
                return &ModulesCache[i].Entry;
        }

        MODULEENTRY32 ModuleEntry;
        ModuleEntry.dwSize = sizeof(ModuleEntry);

        HANDLE Snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, PID);
        if (Snapshot == INVALID_HANDLE_VALUE)
            //ErrorExit("Invalid module snapshot");

        if (!Module32First(Snapshot, &ModuleEntry))
        {
            CloseHandle(Snapshot);
            //ErrorExit("Failed to get first module");
        }

        do {
            if (!strcmp((const char*)ModuleEntry.szModule, szMod))
            {
                CloseHandle(Snapshot);
                ModulesCache.push_back({ ModuleEntry, nullptr });
                return &ModulesCache.back().Entry;
            }
        } while (Module32Next(Snapshot, &ModuleEntry));

        CloseHandle(Snapshot);
        //ErrorExit("Failed to get a module");
    }

    static inline bool IsProcessRunning()
    {
        DWORD ExitCode;
        if (!GetExitCodeProcess(Process, &ExitCode))
        {
            std::clog << "Warning: GetExitCodeProcess failed.\n";
            return true;
        }

        return (ExitCode == STILL_ACTIVE);
    }

    template <typename T>
    inline void Read(uintptr_t address, T &buffer)
    {
        ReadProcessMemory(Process, reinterpret_cast<LPCVOID>(address), &buffer, sizeof(T), 0);
    }

    template <typename T>
    inline T Read(uintptr_t address)
    {
        T buffer;
        ReadProcessMemory(Process, reinterpret_cast<LPCVOID>(address), &buffer, sizeof(T), 0);
        return buffer;
    }

    template <typename T>
    inline void Write(uintptr_t address, const T &value)
    {
        WriteProcessMemory(Process, reinterpret_cast<LPVOID>(address), &value, sizeof(T), 0);
    }

    static const char* GetDir()
    {
        if (!Dir[0])
        {
            DWORD dwDirSize = sizeof(Dir);
            //if (QueryFullProcessImageName(Process, 0, Dir, &dwDirSize) == 0)
                //ErrorExit("Failed to get the game's dir");
            // don't remove trailing slash
            *(strrchr(Dir, '\\') + 1) = '\0';
        }
        return Dir;
    }

    static inline LPVOID GetAlloc()
    {
        if (!Alloc)
        {
            Alloc = VirtualAllocEx(Process, 0, g_AllocSize, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
            if (!Alloc)
                std::clog << "Warning: VirtualAllocEx failed.\n";
        }
        return Alloc;
    }

    static inline void CreateThread(uintptr_t address, LPVOID parameter = 0)
    {
        HANDLE hThread = CreateRemoteThread(Process, 0, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(address), parameter, 0, 0);
        if (!hThread)
        {
            std::clog << "Warning: CreateRemoteThread failed.\n";
            return;
        }
        WaitForSingleObject(hThread, 5000);
        CloseHandle(hThread);
    }

    static inline DWORD CreateThreadReturn(uintptr_t address, LPVOID parameter = 0)
    {
        HANDLE hThread = CreateRemoteThread(Process, 0, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(address), parameter, 0, 0);
        if (!hThread)
        {
            std::clog << "Warning: CreateRemoteThread failed.\n";
            return 0;
        }
        WaitForSingleObject(hThread, 5000);
        DWORD ret = 0;
        GetExitCodeThread(hThread, &ret);
        CloseHandle(hThread);
        return ret;
    }
    static void FindLongestArray(const char* Signature, const char* Mask, int Out[2])
    {
        int startIndex = 0,
            endIndex = strlen(Signature),
            maskLen = strlen(Mask);

        for (int i = endIndex; i < maskLen; ++i)
        {
            if (Mask[i] != 'x')
                continue;

            int count = 0;
            for (int j = i; j < maskLen && Mask[j] == 'x'; ++j)
                ++count;

            if (count > endIndex)
            {
                startIndex = i;
                endIndex = count;
            }

            i += (count - 1);
        }

        Out[0] = startIndex;
        Out[1] = endIndex;
    }

    static uintptr_t FindSignature(const char* Module, const char* Signature, const char* Mask, const ptrdiff_t Offset = 0)
    {
        CModCache* Cache = nullptr;
        for (size_t i = 0; i < ModulesCache.size(); ++i)
        {
            if (!strcmp((const char*)ModulesCache[i].Entry.szModule, Module))
                Cache = &ModulesCache[i];
        }

        if (!Cache)
        {
            GetModuleEntry(Module);
            Cache = &ModulesCache.back();
        }

        if (!Cache->Content)
        {
            Cache->Content = new uint8_t[Cache->Entry.modBaseSize];
            ReadProcessMemory(Process, Cache->Entry.modBaseAddr, Cache->Content, Cache->Entry.modBaseSize, 0);
        }

        int d[2] = { 0 };
        FindLongestArray(Signature, Mask, d);

        const uint8_t len = (uint8_t)strlen(Mask),
            mbeg	= (uint8_t)d[0],
            mlen	= (uint8_t)d[1],
            mfirst	= (uint8_t)Signature[mbeg];

        bool arrInSig[UCHAR_MAX + 1] = { false };

        for (auto i = mbeg; i < mbeg + mlen; ++i)
            arrInSig[(uint8_t)Signature[i]] = true;

        for (int i = Cache->Entry.modBaseSize - len; i >= 0; --i)
        {
            const uint8_t cur = Cache->Content[i];
            bool bInSig = arrInSig[cur];
            bool bSkipped = false;

            while (!bInSig && i > mlen)
            {
                i -= mlen;
                bInSig = arrInSig[Cache->Content[i]];
                bSkipped = true;
            }

            if (bSkipped)
            {
                ++i;
                continue;
            }

            if (cur != mfirst)
                continue;

            if ((i - mbeg < 0) || (i - mbeg + len > Cache->Entry.modBaseSize))
                std::cout << "failed to find a sig" << std::endl;

            for (int j = 0; j < len; ++j)
            {
                if (j == mbeg || Mask[j] != 'x')
                    continue;

                if (Cache->Content[i - mbeg + j] != (uint8_t)Signature[j])
                    break;

                if (j + 1 == len)
                    return Offset ? *(uintptr_t*)(Cache->Content + i - mbeg + Offset) : (uintptr_t)Cache->Entry.modBaseAddr + i - mbeg;
            }
        }

        //ErrorExit("Failed to find a signature");
    }

    static void UnallocContentCache()
    {
        for (size_t i = 0; i < ModulesCache.size(); ++i)
            delete[] ModulesCache[i].Content;
    }
}
