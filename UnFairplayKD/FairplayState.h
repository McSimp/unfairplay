#ifndef FAIRPLAYSTATE_H
#define FAIRPLAYSTATE_H

#include <ntifs.h>
#include "TypeMap.h"

typedef struct
{
    DWORD32 Version;
    DWORD32 DebugBufferLength;
    DWORD32 MaxDebugBufferLength;

    DWORD32 SSDTHooksEnabled; // Bool but 4 bytes
    DWORD32 ObRegisterCallbacksEnabled; // Bool but 4 bytes
    DWORD32 UnknownSSDTCount1;
    DWORD32 UnknownSSDTCount2;
    DWORD32 UNUSED; // 28
    DWORD64 pFunc;

    DWORD32 GTASACount;
    NoTypeEntry GTASAProcs[3];

    DWORD32 MTACount;
    NoTypeEntry MTAProcs[3];
} FairplayState;

#endif