#ifndef TYPEMAP_H
#define TYPEMAP_H

#include <wdm.h>

#define TYPEMAP_MAX_SIZE 100

typedef enum {
    TYPE_NOT_FOUND, 
    TYPE_UNKNOWN,
    TYPE_CSRSS,
    TYPE_LSASS,
    TYPE_SVCHOST,
    TYPE_MTA,
    TYPE_GTASA,
    TYPE_VOIP,
    TYPE_LOGITECH,
    TYPE_RECORDING
} ProcessType;

typedef struct {
    HANDLE ProcessID;
    ProcessType Type;
    BOOLEAN IsManual;
} TypeMapEntry;

typedef struct {
    TypeMapEntry Entries[TYPEMAP_MAX_SIZE];
    INT Count;
    BOOLEAN OverflowReported;
} TypeMap;

void TypeMap_RemoveProcess(HANDLE processID);
void TypeMap_AddEntry(HANDLE processID, ProcessType type);
TypeMapEntry* TypeMap_GetProcessEntry(HANDLE processID);
ProcessType TypeMap_GetProcessType(HANDLE processID);
BOOLEAN TypeMap_HasProcess(HANDLE processID);
INT TypeMap_TypeCount(ProcessType type);
BOOLEAN TypeMap_IsProcessManual(HANDLE processID);

#endif
