#ifndef TYPEMAP_H
#define TYPEMAP_H

#include <ntifs.h>

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

typedef enum {
    SOURCE_AUTO,
    SOURCE_MANUAL,
    SOURCE_CHECK,
    SOURCE_CREATEPROC
} OperationSource;

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

void TypeMap_RemoveEntry(HANDLE processID); // NOPE
void TypeMap_AddEntry(HANDLE processID, ProcessType type); // NOPE
TypeMapEntry* TypeMap_GetProcessEntry(HANDLE processID); // NOPE
ProcessType TypeMap_GetProcessType(HANDLE processID); 
BOOLEAN TypeMap_HasProcess(HANDLE processID); // NOPE
INT TypeMap_TypeCount(ProcessType type);
BOOLEAN TypeMap_IsProcessManual(HANDLE processID);
void TypeMap_LogOperation(const char* operation, HANDLE processID, ProcessType type, OperationSource source, INT deltaCount); // NOPE

void RemoveProcess(HANDLE processID, OperationSource source);
void AddProcess(HANDLE processID, ProcessType type, OperationSource source);

#endif
