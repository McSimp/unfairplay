#ifndef COMMANDS_H
#define COMMANDS_H

#include <ntifs.h>

#define CMD_HANDLER(name) NTSTATUS name(PCHAR pInBuffer, PCHAR pData, ULONG DataLength, PCHAR pOutBuffer, ULONG OutBufferLength, ULONG* pDataWritten)

CMD_HANDLER(CMD_7E_GetState);

NTSTATUS CommandRouter(PCHAR pInBuffer, ULONG InBufferLength, PCHAR pOutBuffer, ULONG OutBufferLength, ULONG* pDataWritten);

#endif