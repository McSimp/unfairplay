#ifndef PROCESSTRAKCER_H
#define PROCESSTRAKCER_H

#include <ntifs.h>
#include "TypeMap.h"

VOID SetNotifyRoutines(PDRIVER_OBJECT pDriverObject);
VOID RemoveNotifyRoutines(PDRIVER_OBJECT pDriverObject);
VOID CreateProcessNotify(HANDLE ParentId, HANDLE ProcessId, BOOLEAN Create);
ProcessType ClassifyProcess(HANDLE ProcessId);
BOOLEAN CompareProcess(PUNICODE_STRING pProcessName, PCUNICODE_STRING pCompareName);

#endif