#include "ProcessTracker.h"
#include "Util.h"

VOID SetNotifyRoutines(PDRIVER_OBJECT pDriverObject)
{
    UNREFERENCED_PARAMETER(pDriverObject);

    LogMessage("[UnFairplay] SetNotifyRoutines\n");

    PsSetCreateProcessNotifyRoutine((PCREATE_PROCESS_NOTIFY_ROUTINE)CreateProcessNotify, FALSE);
    // TODO: ObRegisterCallbacks
}

VOID RemoveNotifyRoutines(PDRIVER_OBJECT pDriverObject)
{
    UNREFERENCED_PARAMETER(pDriverObject);

    LogMessage("[UnFairplay] RemoveNotifyRoutines\n");

    PsSetCreateProcessNotifyRoutine((PCREATE_PROCESS_NOTIFY_ROUTINE)CreateProcessNotify, TRUE);
    // TODO: ObRegisterCallbacks
}

VOID CreateProcessNotify(HANDLE ParentId, HANDLE ProcessId, BOOLEAN Create)
{
    DECLARE_UNICODE_STRING_SIZE(parentName, 600);
    DECLARE_UNICODE_STRING_SIZE(processName, 600);
    ProcessType type;

    LogMessage("[UnFairplay] CreateProcessNotify: ParentId = %d, ProcessId = %d, Create = %d\n", ParentId, ProcessId, Create);

    if (Create)
    {
        if (TypeMap_GetProcessType(ParentId) == TYPE_MTA)
        {
            if (TypeMap_TypeCount(TYPE_MTA) > 1 || TypeMap_TypeCount(TYPE_GTASA))
            {
                GetProcessImageName(ParentId, &parentName);
                GetProcessImageName(ProcessId, &processName);

                LogMessage("Launcher %d(%wZ) creating process %d(%wZ)\n", ParentId, &parentName, ProcessId, &processName);
            }

            AddProcess(ProcessId, TYPE_MTA, SOURCE_CREATEPROC);
        }

        type = TypeMap_GetProcessType(ProcessId);
        if (type == TYPE_NOT_FOUND)
        {
            type = ClassifyProcess(ProcessId);
            if (type != TYPE_UNKNOWN)
            {
                AddProcess(ProcessId, type, SOURCE_AUTO);
            }
        }
    }
    else
    {
        RemoveProcess(ProcessId, SOURCE_AUTO);
    }
}

BOOLEAN CompareProcess(PUNICODE_STRING pProcessName, PCUNICODE_STRING pCompareName)
{
    UNICODE_STRING newProcessName;
    UNICODE_STRING newCompareName;

    // Only compare the last pCompareName->Length characters of pProcessName
    if (pCompareName->Length <= pProcessName->Length)
    {
        newCompareName.Buffer = pCompareName->Buffer;
        newCompareName.Length = pCompareName->Length;
        newCompareName.MaximumLength = pCompareName->Length;

        newProcessName.Buffer = pProcessName->Buffer + ((pProcessName->Length - pCompareName->Length) / 2);
        newProcessName.Length = pCompareName->Length;
        newProcessName.MaximumLength = pCompareName->Length;

        return RtlCompareUnicodeString(&newProcessName, &newCompareName, TRUE) == 0;
    }
    else
    {
        return FALSE;
    }
}

DECLARE_GLOBAL_CONST_UNICODE_STRING(GTASAProcess, L"gta_sa.exe");
DECLARE_GLOBAL_CONST_UNICODE_STRING(ProxySAProccess, L"proxy_sa.exe");
DECLARE_GLOBAL_CONST_UNICODE_STRING(MTAShortProcess, L"MULTIT~1.EXE");
DECLARE_GLOBAL_CONST_UNICODE_STRING(MTAProcess, L"Multi Theft Auto.exe");
DECLARE_GLOBAL_CONST_UNICODE_STRING(CSRSSProcess, L"csrss.exe");
DECLARE_GLOBAL_CONST_UNICODE_STRING(LSASSProcess, L"lsass.exe");
DECLARE_GLOBAL_CONST_UNICODE_STRING(SvchostProcess, L"svchost.exe");
DECLARE_GLOBAL_CONST_UNICODE_STRING(RaidcallProcess, L"raidcall.exe");
DECLARE_GLOBAL_CONST_UNICODE_STRING(LVPrcSrvProcess, L"LVPrcSrv.exe");
DECLARE_GLOBAL_CONST_UNICODE_STRING(LWEMonProcess, L"LWEMon.exe");
DECLARE_GLOBAL_CONST_UNICODE_STRING(Actionx64Process, L"Action_x64.bin");
DECLARE_GLOBAL_CONST_UNICODE_STRING(Actionx86Process, L"Action_x86.bin");

ProcessType ClassifyProcess(HANDLE ProcessId)
{
    DECLARE_UNICODE_STRING_SIZE(processName, 600);

    GetProcessImageName(ProcessId, &processName);

    if (CompareProcess(&processName, &GTASAProcess)
        || CompareProcess(&processName, &ProxySAProccess))
    {
        return TYPE_GTASA;
    }
        
    if (CompareProcess(&processName, &MTAShortProcess)
        || CompareProcess(&processName, &MTAProcess))
    {
        return TYPE_MTA;
    }

    if (CompareProcess(&processName, &CSRSSProcess))
        return TYPE_CSRSS;

    if (CompareProcess(&processName, &LSASSProcess))
        return TYPE_LSASS;

    if (CompareProcess(&processName, &SvchostProcess))
        return TYPE_SVCHOST;

    if (CompareProcess(&processName, &RaidcallProcess))
        return TYPE_VOIP;

    if (CompareProcess(&processName, &LVPrcSrvProcess)
        || CompareProcess(&processName, &LWEMonProcess))
    {
        return TYPE_LOGITECH;
    }

    if (CompareProcess(&processName, &Actionx64Process)
        || CompareProcess(&processName, &Actionx86Process))
    {
        return TYPE_RECORDING;
    }

    LogMessage("[UnFairplay] Unknown process type\n");
    return TYPE_UNKNOWN;
}
