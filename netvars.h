#include <windows.h>
#include "Structs.h"
#include "Functions.h"

struct netVars{
    uintptr_t m_clrRender;
    uintptr_t m_iTeamNum;
    uintptr_t m_vecOrigin;
    uintptr_t m_nModelIndex;
    uintptr_t m_pBones;
};

class NetVars
{
public:
    static void init();
    static uintptr_t getNetVar(const char* tablename, const char* netvarname);
    static netVars getAllNetVars();
};
