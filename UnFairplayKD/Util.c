#pragma warning(disable:4055)
#include "Util.h"

#define INVALID_HANDLE_VALUE (HANDLE)-1

QUERY_INFO_PROCESS ZwQueryInformationProcess = NULL;

QUERY_INFO_PROCESS GetZwQueryInformationProcess()
{
    if (ZwQueryInformationProcess != NULL)
    {
        return ZwQueryInformationProcess;
    }

    UNICODE_STRING routineName;
    RtlInitUnicodeString(&routineName, L"ZwQueryInformationProcess");
    ZwQueryInformationProcess = (QUERY_INFO_PROCESS)MmGetSystemRoutineAddress(&routineName);
    
    LogMessage("[UnFairplay] ZwQueryInformationProcess = 0x%X\r\n", GetZwQueryInformationProcess);

    return ZwQueryInformationProcess;
}

HANDLE GetProcessInformation(HANDLE processID)
{
    NTSTATUS status;
    PEPROCESS process;
    HANDLE processHandle;

    status = PsLookupProcessByProcessId(processID, &process);

    if (NT_SUCCESS(status))
    {
        ObOpenObjectByPointer(process, 0, NULL, 0, NULL, KernelMode, &processHandle);
        ObDereferenceObject(process);
        return processHandle;
    }
    else
    {
        LogMessage("[UnFairplay] GetProcessInformation failed (0x%X)\r\n", status);
        return INVALID_HANDLE_VALUE;
    }
}

HANDLE GetUniqueProcessID(HANDLE processHandle)
{
    NTSTATUS status;
    QUERY_INFO_PROCESS qip;
    PROCESS_BASIC_INFORMATION pbi;
    ULONG returnedLength;

    qip = GetZwQueryInformationProcess();

    if (qip == NULL)
    {
        return INVALID_HANDLE_VALUE;
    }

    status = qip(processHandle, ProcessBasicInformation, (PVOID)&pbi, sizeof(pbi), &returnedLength);

    if (!NT_SUCCESS(status))
    {
        LogMessage("[UnFairplay] GetUniqueProcessID failed (0x%X)\r\n", status);
        return INVALID_HANDLE_VALUE;
    }

    LogMessage("[UnFairplay] UniqueProcessId = %d\r\n", pbi.UniqueProcessId);

    return (HANDLE)pbi.UniqueProcessId;
}

NTSTATUS QueryProcessImageFileName(HANDLE processHandle, PUNICODE_STRING outputString)
{
    NTSTATUS status;
    QUERY_INFO_PROCESS qip;
    ULONG returnedLength;
    WCHAR nameBuffer[(sizeof(UNICODE_STRING) / sizeof(WCHAR)) + 260];
    PUNICODE_STRING imageName;

    qip = GetZwQueryInformationProcess();

    if (qip == NULL)
    {
        return ERROR_SEVERITY_ERROR;
    }

    status = qip(processHandle, ProcessImageFileName, &nameBuffer, sizeof(nameBuffer), &returnedLength);

    if (NT_SUCCESS(status))
    {
        imageName = (PUNICODE_STRING)nameBuffer;
        RtlCopyUnicodeString(outputString, imageName);
    }
    else
    {
        LogMessage("[UnFairplay] QueryProcessImageFileName failed (0x%X)\r\n", status);
    }

    // TODO: Fairplay does some transformations on the string here

    return status;
}

NTSTATUS GetProcessImageName(HANDLE processID, PUNICODE_STRING processImageName)
{
    NTSTATUS status;
    HANDLE processHandle;
    HANDLE uniqueProcID;

    // Initialize image name string
    processImageName->Length = 0;
    RtlZeroMemory(processImageName->Buffer, processImageName->MaximumLength);

    processHandle = GetProcessInformation(processID);
    uniqueProcID = GetUniqueProcessID(processHandle);

    if (uniqueProcID == processID)
    {
        LogMessage("[UnFairplay] uniqueProcID == processID (%d, %d)\r\n", uniqueProcID, processID);
        status = QueryProcessImageFileName(processHandle, processImageName);
        LogMessage("[UnFairplay] QueryProcessImageFileName status = 0x%X\r\n", status);
        ZwClose(processHandle);
        return status;
    }
    else
    {
        LogMessage("[UnFairplay] uniqueProcID != processID (%d, %d)\r\n", uniqueProcID, processID);
        ZwClose(processHandle);
        return ERROR_SEVERITY_ERROR;
    }
}
