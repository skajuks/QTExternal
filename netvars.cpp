#include "netvars.h"

RecvTable* getRecvTable(const char* name){
    ClientClass cc;
    Memory.read<ClientClass>(pOffsets.g_pClientClassHead, cc);

    while(true){
        char var[64];
        Memory.read<char[64]>(reinterpret_cast<uintptr_t>(cc.m_pNetworkName), var);

        if(!strcmp(var, name)){
            std::cout << cc.m_pRecvTable->m_pNetTableName << std::endl;
            return cc.m_pRecvTable;
        }
        if(cc.m_pNext == 0)
            break;

        Memory.read<ClientClass>(reinterpret_cast<uintptr_t>(cc.m_pNext), cc);
    }
    return nullptr;
}

int getProp(RecvTable* pTable, const char* name){

    RecvTable table;
    Memory.read<RecvTable>(reinterpret_cast<uintptr_t>(pTable), table);

    int offset = 0;
    std::cout << table.m_nProps << std::endl;
    for(int i = 0; i < table.m_nProps; i++){
        RecvProp prop;
        Memory.read<RecvProp>((uintptr_t)(table.m_nProps) + i * sizeof(RecvProp), prop);
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

uintptr_t NetVars::getNetVar(const char* tablename, const char* netvarname){

    RecvTable* table = getRecvTable(tablename);
    if(!table){
        std::cout << "Error, failed to find netvar [ table ]" << std::endl;
        return 0;
    }
    int offset = getProp(table, netvarname);
    if(!offset){
        std::cout << "Error, failed to find netvar [ offset ]" << std::endl;
        return 0;
    }
    return offset;
}

netVars NetVars::getAllNetVars(){
    netVars vars;
    vars.m_clrRender = getNetVar("CBaseEntity", "m_iTeamNum");


    std::cout << vars.m_clrRender << std::endl;

    return vars;
}
