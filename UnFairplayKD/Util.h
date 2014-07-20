#ifndef UTIL_H
#define UTIL_H

#include <ntifs.h>

#define LogMessage DbgPrint

typedef NTSTATUS(__stdcall *QUERY_INFO_PROCESS)(
    __in       HANDLE ProcessHandle,
    __in       PROCESSINFOCLASS ProcessInformationClass,
    __out      PVOID ProcessInformation,
     __in      ULONG ProcessInformationLength,
    __out_opt  PULONG ReturnLength
);

QUERY_INFO_PROCESS GetZwQueryInformationProcess();
HANDLE GetProcessInformation(HANDLE processID);
HANDLE GetUniqueProcessID(HANDLE processHandle);
NTSTATUS QueryProcessImageFileName(HANDLE processHandle, PUNICODE_STRING outputString);
NTSTATUS GetProcessImageName(HANDLE processID, PUNICODE_STRING processImageName);

#endif