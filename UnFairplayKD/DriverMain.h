#ifndef DRIVERMAIN_H
#define DRIVERMAIN_H

#include <wdm.h>
#include <Ntstrsafe.h>
#define LogMessage DbgPrint

NTSTATUS DriverEntry(PDRIVER_OBJECT pDriverObject, PUNICODE_STRING pRegistryPath);
VOID DriverUnload(PDRIVER_OBJECT pDriverObject);

NTSTATUS IRPUnsupported(PDEVICE_OBJECT pDeviceObject, PIRP pIrp);
NTSTATUS IRPIgnore(PDEVICE_OBJECT pDeviceObject, PIRP pIrp);
NTSTATUS IRPControl(PDEVICE_OBJECT pDeviceObject, PIRP pIrp);

NTSTATUS CommandHandler(PCHAR pInBuffer, ULONG inBufferLength, PCHAR pOutBuffer, ULONG outBufferLength, ULONG* pDataWritten);

#endif