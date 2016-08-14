#include "DriverMain.h"
#include <Ntstrsafe.h>
#include "Util.h"
#include "ProcessTracker.h"
#include "Commands.h"

#define IOCTL_FAIRPLAY_CONTROL_CODE          \
        CTL_CODE(FILE_DEVICE_UNKNOWN,        \
                 0x802,                      \
                 METHOD_BUFFERED,            \
                 FILE_READ_DATA | FILE_WRITE_DATA)

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(PAGE, DriverUnload)
#endif

NTSTATUS DriverEntry(PDRIVER_OBJECT pDriverObject, PUNICODE_STRING pRegistryPath)
{
	UNICODE_STRING deviceName, dosDeviceName;
	NTSTATUS NtStatus;
	PDEVICE_OBJECT pDeviceObject = NULL;
    INT i;

	UNREFERENCED_PARAMETER(pRegistryPath);

	LogMessage("[UnFairplay] Kernel driver initialization\r\n");

    RtlUnicodeStringInit(&deviceName, L"\\Device\\FairplayKD0");
    RtlUnicodeStringInit(&dosDeviceName, L"\\DosDevices\\FairplayKD0");

	// Create device for communicating with user land
	NtStatus = IoCreateDevice(
		pDriverObject, 
		0, 
		&deviceName,
		FILE_DEVICE_UNKNOWN,
        0, // Note: Actual Fairplay driver uses 0 here
		FALSE, 
		&pDeviceObject);

    if (!NT_SUCCESS(NtStatus))
    {
        return NtStatus;
    }

    if (!pDeviceObject)
    {
        return STATUS_UNEXPECTED_IO_ERROR;
    }

    // Setup IRP functions
    for (i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; i++)
    {
        pDriverObject->MajorFunction[i] = IRPUnsupported;
    }

    pDriverObject->MajorFunction[IRP_MJ_CREATE] = IRPIgnore;
    pDriverObject->MajorFunction[IRP_MJ_CLOSE] = IRPIgnore;
    pDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = IRPControl;

    pDriverObject->DriverUnload = DriverUnload;

    // Fairplay uses buffered IO
    pDeviceObject->Flags |= DO_BUFFERED_IO;
    pDeviceObject->AlignmentRequirement = TRUE;

    IoCreateSymbolicLink(&dosDeviceName, &deviceName);

    pDeviceObject->Flags &= (~DO_DEVICE_INITIALIZING);

    SetNotifyRoutines(pDriverObject);

    return STATUS_SUCCESS;
}

VOID DriverUnload(PDRIVER_OBJECT pDriverObject)
{
	UNICODE_STRING dosDeviceName;

	LogMessage("[UnFairplay] Shutting down driver\r\n");

    RemoveNotifyRoutines(pDriverObject);

    RtlUnicodeStringInit(&dosDeviceName, L"\\DosDevices\\FairplayKD0");
	IoDeleteSymbolicLink(&dosDeviceName);
	IoDeleteDevice(pDriverObject->DeviceObject);
}

NTSTATUS IRPUnsupported(PDEVICE_OBJECT pDeviceObject, PIRP pIrp)
{
	UNREFERENCED_PARAMETER(pDeviceObject);

	LogMessage("[UnFairplay] IRPUnsupported called\r\n");

	pIrp->IoStatus.Information = 0;
	pIrp->IoStatus.Status = STATUS_NOT_SUPPORTED;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);

	return pIrp->IoStatus.Status;
}

NTSTATUS IRPIgnore(PDEVICE_OBJECT pDeviceObject, PIRP pIrp)
{
	UNREFERENCED_PARAMETER(pDeviceObject);

	LogMessage("[UnFairplay] IRPIgnore called\r\n");

	pIrp->IoStatus.Information = 0;
	pIrp->IoStatus.Status = STATUS_SUCCESS;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return pIrp->IoStatus.Status;
}

NTSTATUS IRPControl(PDEVICE_OBJECT pDeviceObject, PIRP pIrp)
{
    NTSTATUS NtStatus = STATUS_NOT_SUPPORTED;
    PIO_STACK_LOCATION pIoStackIrp = NULL;
    ULONG dataWritten = 0;

	UNREFERENCED_PARAMETER(pDeviceObject);

    LogMessage("[UnFairplay] IRPControl called\r\n");

    pIoStackIrp = IoGetCurrentIrpStackLocation(pIrp);    

    if(pIoStackIrp && pIoStackIrp->Parameters.DeviceIoControl.IoControlCode == IOCTL_FAIRPLAY_CONTROL_CODE)
    {
		NtStatus = CommandHandler(
			(PCHAR)pIrp->AssociatedIrp.SystemBuffer, // Input Buffer
			pIoStackIrp->Parameters.DeviceIoControl.InputBufferLength,
			(PCHAR)pIrp->AssociatedIrp.SystemBuffer, // Output Buffer
			pIoStackIrp->Parameters.DeviceIoControl.OutputBufferLength,
			&dataWritten
		);
    }

    LogMessage("[UnFairplay] dw = %d, stat = 0x%X\r\n", dataWritten, NtStatus);

    pIrp->IoStatus.Status = NtStatus;
    pIrp->IoStatus.Information = 100;

    IoCompleteRequest(pIrp, IO_NO_INCREMENT);

    return NtStatus;
}

NTSTATUS CommandHandler(PCHAR pInBuffer, ULONG InBufferLength, PCHAR pOutBuffer, ULONG OutBufferLength, ULONG* pDataWritten)
{
	if(!pInBuffer || !pOutBuffer)
		return STATUS_UNSUCCESSFUL;

	LogMessage("[UnFairplay] CommandHandler called\r\n");

    return CommandRouter(pInBuffer, InBufferLength, pOutBuffer, OutBufferLength, pDataWritten);
}
