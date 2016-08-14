#ifndef DRIVERMAIN_H
#define DRIVERMAIN_H

#include <ntifs.h>

NTSTATUS DriverEntry(PDRIVER_OBJECT pDriverObject, PUNICODE_STRING pRegistryPath);
VOID DriverUnload(PDRIVER_OBJECT pDriverObject);

NTSTATUS IRPUnsupported(PDEVICE_OBJECT pDeviceObject, PIRP pIrp);
NTSTATUS IRPIgnore(PDEVICE_OBJECT pDeviceObject, PIRP pIrp);
NTSTATUS IRPControl(PDEVICE_OBJECT pDeviceObject, PIRP pIrp);

NTSTATUS CommandHandler(PCHAR pInBuffer, ULONG InBufferLength, PCHAR pOutBuffer, ULONG OutBufferLength, ULONG* pDataWritten);

#endif