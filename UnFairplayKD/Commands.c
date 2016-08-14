#include "Commands.h"
#include "FairplayState.h"
#include "Util.h"
#include "FairplayConst.h"

void WriteProcessState(FairplayState* state)
{
    INT i;

    state->GTASACount = TypeMap_TypeCount(TYPE_GTASA);
    for (i = 0; i < 3; i++)
    {
        state->GTASAProcs[i] = TypeMap_GetNoTypeEntry(TYPE_GTASA, i + 1);
    }

    state->MTACount = TypeMap_TypeCount(TYPE_MTA);
    for (i = 0; i < 3; i++)
    {
        state->MTAProcs[i] = TypeMap_GetNoTypeEntry(TYPE_MTA, i + 1);
    }

    state->ObRegisterCallbacksEnabled = 1;
    state->pFunc = (DWORD64)&WriteProcessState;
}

void WriteSSDTState(FairplayState* state)
{
    state->SSDTHooksEnabled = 0;
    state->UnknownSSDTCount1 = 0;
    state->UnknownSSDTCount2 = 0;
}

ULONG ComputeChecksum(PCHAR pBuffer, ULONG Length)
{
    ULONG result = 0x4661223;
    ULONG i;

    for (i = 0; i < Length; i++)
    {
        result = (result + pBuffer[i]) ^ (pBuffer[i] << ((i & 7) + 8));
    }
    return result;
}

void AppendDataToBuffer(const void* pData, size_t DataLength, PCHAR pOutBuffer, ULONG OutBufferLength, ULONG* pDataWritten)
{
    ULONG dataWritten = *pDataWritten;

    if (OutBufferLength - dataWritten >= DataLength)
    {
        memcpy(pOutBuffer + dataWritten, pData, DataLength);
        *pDataWritten = dataWritten + 4;
    }
}

void AppendChecksum(PCHAR pOutBuffer, ULONG OutBufferLength, ULONG* pDataWritten)
{
    ULONG checksum;
    
    checksum = ComputeChecksum(pOutBuffer, *pDataWritten);
    AppendDataToBuffer(&checksum, 4, pOutBuffer, OutBufferLength, pDataWritten);
}

BOOLEAN VerifyCommandChecksum(PCHAR pInBuffer, ULONG InBufferLength)
{
    if (InBufferLength < 4)
    {
        return FALSE;
    }
    else
    {
        return ComputeChecksum(pInBuffer, InBufferLength - 4) == *(ULONG*)(pInBuffer + InBufferLength - 4);
    }
}

CMD_HANDLER(CMD_7E_GetState)
{
    UNREFERENCED_PARAMETER(DataLength);
    UNREFERENCED_PARAMETER(pData);
    UNREFERENCED_PARAMETER(pInBuffer);

    FairplayState state;
    memset(&state, 0, sizeof(state));

    if (OutBufferLength >= 100)
    {
        state.DebugBufferLength = 0;
        state.MaxDebugBufferLength = DEBUG_BUFFER_MAX;

        WriteProcessState(&state);
        WriteSSDTState(&state);

        state.Version = FAIRPLAY_VERSION;

        memcpy(pOutBuffer, &state, sizeof(state));
        *pDataWritten = sizeof(state);

        AppendChecksum(pOutBuffer, OutBufferLength, pDataWritten);

        return STATUS_SUCCESS;
    }
    else
    {
        return STATUS_BUFFER_TOO_SMALL;
    }
}

NTSTATUS CommandRouter(PCHAR pInBuffer, ULONG InBufferLength, PCHAR pOutBuffer, ULONG OutBufferLength, ULONG* pDataWritten)
{
    INT commandNum;
    ULONG parsedVersion;

    if (InBufferLength < 4)
    {
        LogMessage("[UnFairplay] Command failed\r\n");
        return STATUS_INVALID_SYSTEM_SERVICE;
    }

    commandNum = *(INT*)pInBuffer;
    LogMessage("[UnFairplay] Command = %d\r\n", commandNum);

    if (commandNum == 0x7E)
    {
        if (InBufferLength >= 12)
        {
            parsedVersion = *(ULONG*)(pInBuffer + 4);

            if (parsedVersion == FAIRPLAY_VERSION)
            {
                if (InBufferLength == 12 && VerifyCommandChecksum(pInBuffer, 12))
                {
                    return CMD_7E_GetState(pInBuffer, pInBuffer + 8, 0, pOutBuffer, OutBufferLength, pDataWritten);
                }
            }
        }
    }

    return STATUS_INVALID_SYSTEM_SERVICE;
}
