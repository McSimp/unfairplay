#include "TypeMap.h"
#include "DriverMain.h"

static TypeMap g_TypeMap;

void TypeMap_RemoveProcess(HANDLE processID)
{
    INT i;
    for (i = 0; i < g_TypeMap.Count; i++)
    {
        if (g_TypeMap.Entries[i].ProcessID == processID)
        {
            // Shuffle the entries after our target up if needed
            INT needsShuffle = g_TypeMap.Count - i - 1;
            
            if (needsShuffle > 0)
            {
                memmove(g_TypeMap.Entries + i, g_TypeMap.Entries + i + 1, sizeof(TypeMapEntry) * needsShuffle);
            }
            
            // TODO: Fairplay implementation does not short circuit, but probably should?
            g_TypeMap.Count--;
            i--;
        }
    }
}

void TypeMap_AddEntry(HANDLE processID, ProcessType type)
{
    if (type == TYPE_NOT_FOUND)
    {
        return;
    }

    // Remove any existing entries for this process
    TypeMap_RemoveProcess(processID);

    if (g_TypeMap.Count >= TYPEMAP_MAX_SIZE)
    {
        if (!g_TypeMap.OverflowReported)
        {
            g_TypeMap.OverflowReported = TRUE;
            LogMessage("TypeMap overflow\r\n");
        }
    }
    else
    {
        TypeMapEntry* entry = &g_TypeMap.Entries[g_TypeMap.Count];
        entry->ProcessID = processID;
        entry->Type = type;
        entry->IsManual = 0;
        g_TypeMap.Count++;
    }
}

TypeMapEntry* TypeMap_GetProcessEntry(HANDLE processID)
{
    INT i;
    for (i = 0; i < g_TypeMap.Count; i++)
    {
        TypeMapEntry* current = &g_TypeMap.Entries[i];

        if (current->ProcessID == processID)
        {
            return current;
        }
    }

    return NULL;
}

ProcessType TypeMap_GetProcessType(HANDLE processID)
{
    TypeMapEntry* entry = TypeMap_GetProcessEntry(processID);
    if (entry)
    {
        return entry->Type;
    }
    else
    {
        return TYPE_NOT_FOUND;
    }
}

BOOLEAN TypeMap_HasProcess(HANDLE processID)
{
    return TypeMap_GetProcessType(processID) != TYPE_NOT_FOUND;
}

INT TypeMap_TypeCount(ProcessType type)
{
    INT count = 0;
    INT i;

    for (i = 0; i < g_TypeMap.Count; i++)
    {
        if (g_TypeMap.Entries[i].Type == type)
        {
            count++;
        }
    }

    return count;
}

BOOLEAN TypeMap_IsProcessManual(HANDLE processID)
{
    TypeMapEntry* entry = TypeMap_GetProcessEntry(processID);
    if (entry)
    {
        return entry->IsManual;
    }
    else
    {
        return FALSE;
    }
}

void TypeMap_SetManualByType(ProcessType type, BOOLEAN manual)
{
    INT i;
    for (i = 0; i < g_TypeMap.Count; i++)
    {
        TypeMapEntry* current = &g_TypeMap.Entries[i];

        if (current->Type == type)
        {
            current->IsManual = manual;
        }
    }
}
