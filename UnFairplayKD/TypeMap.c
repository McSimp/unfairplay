#include "TypeMap.h"
#include "Util.h"

static TypeMap g_TypeMap;

void TypeMap_RemoveEntry(HANDLE processID)
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
    TypeMap_RemoveEntry(processID);

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

NoTypeEntry TypeMap_GetNoTypeEntry(ProcessType type, INT num)
{
    INT i;
    NoTypeEntry result;

    result.ProcessID = 0;
    result.IsManual = 0;

    for (i = 0; i < g_TypeMap.Count; i++)
    {
        TypeMapEntry* current = &g_TypeMap.Entries[i];

        if (current->Type == type)
        {
            num--;
            if (num == 0)
            {
                result.ProcessID = current->ProcessID;
                result.IsManual = current->IsManual;
            }
        }
    }

    return result;
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

DECLARE_GLOBAL_CONST_UNICODE_STRING(SourceAutoString, L"auto");
DECLARE_GLOBAL_CONST_UNICODE_STRING(SourceManualString, L"manual");
DECLARE_GLOBAL_CONST_UNICODE_STRING(SourceCheckString, L"dur_check");
DECLARE_GLOBAL_CONST_UNICODE_STRING(SourceCreateProcString, L"dur_createproc");
DECLARE_GLOBAL_CONST_UNICODE_STRING(SourceUnknownString, L"unknown");

DECLARE_GLOBAL_CONST_UNICODE_STRING(ProtectedString, L"protected");
DECLARE_GLOBAL_CONST_UNICODE_STRING(LauncherString, L"launcher");

void TypeMap_LogOperation(const char* operation, HANDLE processID, ProcessType type, OperationSource source, INT deltaCount)
{
    DECLARE_UNICODE_STRING_SIZE(processName, 600);
    const UNICODE_STRING* sourceString;
    const UNICODE_STRING* typeString;
    INT count;

    if (type == TYPE_MTA || type == TYPE_GTASA)
    {
        GetProcessImageName(processID, &processName);

        if (source == SOURCE_AUTO)
        {
            sourceString = &SourceAutoString;
        }
        else if (source == SOURCE_MANUAL)
        {
            sourceString = &SourceManualString;
        }
        else if (source == SOURCE_CHECK)
        {
            sourceString = &SourceCheckString;
        }
        else if (source == SOURCE_CREATEPROC)
        {
            sourceString = &SourceCreateProcString;
        }
        else
        {
            sourceString = &SourceUnknownString;
        }

        if (type == TYPE_GTASA)
        {
            typeString = &ProtectedString;
        }
        else
        {
            typeString = &LauncherString;
        }

        count = TypeMap_TypeCount(type) + deltaCount;

        LogMessage("[UnFairplay] %sMapping (%wZ) for %wZ (Total after:%d) - %d(%wZ)\r\n", operation, sourceString, typeString, count, processID, &processName);
    }
}

void RemoveProcess(HANDLE processID, OperationSource source)
{
    ProcessType type;

    LogMessage("[UnFairplay] Removing processID = %d, source = %d\r\n", processID, source);

    if (TypeMap_HasProcess(processID))
    {
        type = TypeMap_GetProcessType(processID);
        TypeMap_LogOperation("Remove", processID, type, source, -1);
        TypeMap_RemoveEntry(processID);
    }
}

void AddProcess(HANDLE processID, ProcessType type, OperationSource source)
{
    ProcessType cachedType;
    
    cachedType = TypeMap_GetProcessType(processID);

    LogMessage("[UnFairplay] Adding processID = %d, type = %d, source = %d, cachedType = %d\r\n", processID, type, source, cachedType);

    if (cachedType != type)
    {
        TypeMap_LogOperation("Add", processID, type, source, 1);
        TypeMap_AddEntry(processID, type);
    }

    if ((type == TYPE_GTASA && source == SOURCE_MANUAL)
        || (type == TYPE_MTA && TypeMap_TypeCount(TYPE_GTASA) > 0))
    {
        TypeMap_SetManualByType(TYPE_GTASA, 1);
    }
}
