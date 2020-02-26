#include "BUFFER_IO.h"

NTSTATUS DriverEntry(IN PDRIVER_OBJECT pDriverObject, IN PUNICODE_STRING pRegistryPath)
{
	NTSTATUS status;
	pDriverObject->DriverUnload = HelloDDKUnload;

	KdPrint(("Enter DriverEntry\n"));
	pDriverObject->MajorFunction[IRP_MJ_CREATE] = HelloDDKDispatchRoutin;
	pDriverObject->MajorFunction[IRP_MJ_CLOSE] = HelloDDKDispatchRoutin;
	pDriverObject->MajorFunction[IRP_MJ_WRITE] = HelloDDKWrite;
	pDriverObject->MajorFunction[IRP_MJ_READ] = HelloDDKRead;
	pDriverObject->MajorFunction[IRP_MJ_CLEANUP] = HelloDDKDispatchRoutin;
	pDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = HelloDDKDispatchRoutin;
	pDriverObject->MajorFunction[IRP_MJ_SET_INFORMATION] = HelloDDKDispatchRoutin;
	pDriverObject->MajorFunction[IRP_MJ_SHUTDOWN] = HelloDDKDispatchRoutin;
	pDriverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL] = HelloDDKDispatchRoutin;
	pDriverObject->MajorFunction[IRP_MJ_QUERY_INFORMATION] = HelloDDKQueryInfomation;

	status = CreateDevice(pDriverObject);

	KdPrint(("Leave DriverEntry\n"));
	return status;
}

NTSTATUS CreateDevice(IN PDRIVER_OBJECT pDriverObject)
{
	NTSTATUS status;
	PDEVICE_OBJECT pDevObj;
	PDEVICE_EXTENSION pDevExt;

	UNICODE_STRING devName;
	RtlInitUnicodeString(&devName, L"\\Device\\MyDDKDevice");
	status = IoCreateDevice(pDriverObject,
		sizeof(DEVICE_EXTENSION),
		&devName,
		FILE_DEVICE_UNKNOWN,
		0, TRUE,
		&pDevObj);
	if (!NT_SUCCESS(status))
		return status;

	pDevObj->Flags |= DO_BUFFERED_IO;
	pDevExt = (PDEVICE_EXTENSION)pDevObj->DeviceExtension;
	pDevExt->pDeviceObj = pDevObj;
	pDevExt->usDeviceName = devName;
	UNICODE_STRING symLinkName;
	RtlInitUnicodeString(&symLinkName, L"\\??\\HelloDDK");
	pDevExt->pBuffer = (PUCHAR)ExAllocatePool(PagedPool, MAX_FILE_LENGTH);
	pDevExt->usSymbolName = symLinkName;
	pDevExt->uFileLen = 0;
	status = IoCreateSymbolicLink(&symLinkName, &devName);
	if (!NT_SUCCESS(status))
	{
		IoDeleteDevice(pDevObj);
		return status;
	}
	return STATUS_SUCCESS;
}

VOID HelloDDKUnload(IN PDRIVER_OBJECT pDriverObject)
{
	PDEVICE_OBJECT pNextDevObj = pDriverObject->DeviceObject;
	KdPrint(("Enter DriverUnload\n"));
	while (pNextDevObj != NULL)
	{
		PDEVICE_EXTENSION pDevExt = pNextDevObj->DeviceExtension;
		IoDeleteSymbolicLink(&pDevExt->usSymbolName);
		if (pDevExt->pBuffer)
		{
			ExFreePool(pDevExt->pBuffer);
			pDevExt->pBuffer = NULL;
		}


		pNextDevObj = pNextDevObj->NextDevice;
		IoDeleteDevice(pDevExt->pDeviceObj);
	}
}

NTSTATUS HelloDDKDispatchRoutin(IN PDEVICE_OBJECT pDevObj, IN PIRP pIrp)
{
	KdPrint(("Enter HelloDDKDispatchRoutin\n"));

	PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(pIrp);
	static char* irpname[] =
	{
		"IRP_MJ_CREATE",
		"IRP_MJ_CREATE_NAMED_PIPE",
		"IRP_MJ_CLOSE",
		"IRP_MJ_READ",
		"IRP_MJ_WRITE",
		"IRP_MJ_QUERY_INFORMATION",
		"IRP_MJ_SET_INFORMATION",
		"IRP_MJ_QUERY_EA",
		"IRP_MJ_SET_EA",
		"IRP_MJ_FLUSH_BUFFERS",
		"IRP_MJ_QUERY_VOLUME_INFORMATION",
		"IRP_MJ_SET_VOLUME_INFORMATION",
		"IRP_MJ_DIRECTORY_CONTROL",
		"IRP_MJ_FILE_SYSTEM_CONTROL",
		"IRP_MJ_DEVICE_CONTROL",
		"IRP_MJ_INTERNAL_DEVICE_CONTROL",
		"IRP_MJ_SHUTDOWN",
		"IRP_MJ_LOCK_CONTROL",
		"IRP_MJ_CLEANUP",
		"IRP_MJ_CREATE_MAILSLOT",
		"IRP_MJ_QUERY_SECURITY",
		"IRP_MJ_SET_SECURITY",
		"IRP_MJ_POWER",
		"IRP_MJ_SYSTEM_CONTROL",
		"IRP_MJ_DEVICE_CHANGE",
		"IRP_MJ_QUERY_QUOTA",
		"IRP_MJ_SET_QUOTA",
		"IRP_MJ_PNP",
	};

	UCHAR type = stack->MajorFunction;
	if (type >= arraysize(irpname))
		KdPrint((" - Unknown IRP, major type %X\n", type));
	else
		KdPrint(("\t%s\n", irpname[type]));

	NTSTATUS status = STATUS_SUCCESS;

	pIrp->IoStatus.Status = status;
	pIrp->IoStatus.Information = 0;	// bytes xfered
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);

	KdPrint(("Leave HelloDDKDispatchRoutin\n"));

	return status;
}

NTSTATUS HelloDDKRead(IN PDEVICE_OBJECT pDevObj, IN PIRP pIrp)
{
	KdPrint(("Enter HelloDDKRead\n"));

	NTSTATUS status = STATUS_SUCCESS;
	PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(pIrp);
	PDEVICE_EXTENSION pDevExt = pDevObj->DeviceExtension;
	ULONG ulReadLength = stack->Parameters.Read.Length;
	ULONG ulReadOffset = stack->Parameters.Read.ByteOffset.QuadPart;
	if (ulReadLength + ulReadOffset > MAX_FILE_LENGTH)
	{
		status = STATUS_FILE_INVALID;
		ulReadLength = 0;
	}
	else
	{
		memcpy(pIrp->AssociatedIrp.SystemBuffer, pDevExt->pBuffer, ulReadLength);
		status = STATUS_SUCCESS;
	}

	pIrp->IoStatus.Status = status;
	pIrp->IoStatus.Information = ulReadLength;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	KdPrint(("Leave HelloDDKRead\n"));

	return status;
}

NTSTATUS HelloDDKWrite(IN PDEVICE_OBJECT pDevObj, IN PIRP pIrp)
{
	KdPrint(("Enter HelloDDKWrite\n"));

	NTSTATUS status = STATUS_SUCCESS;
	PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(pIrp);
	PDEVICE_EXTENSION pDevExt = pDevObj->DeviceExtension;
	ULONG ulWriteLength = stack->Parameters.Write.Length;
	ULONG ulWriteOffset = (ULONG)stack->Parameters.Write.ByteOffset.QuadPart;

	if (ulWriteLength + ulWriteOffset > MAX_FILE_LENGTH)
	{
		status = STATUS_FILE_INVALID;
		ulWriteLength = 0;
	}
	else
	{
		memcpy(pDevExt->pBuffer + ulWriteOffset, pIrp->AssociatedIrp.SystemBuffer, ulWriteLength);
		status = STATUS_SUCCESS;
		if (ulWriteLength + ulWriteOffset > pDevExt->uFileLen)
		{
			pDevExt->uFileLen = ulWriteLength + ulWriteOffset;
		}
	}
	pIrp->IoStatus.Status = status;
	pIrp->IoStatus.Information = ulWriteLength;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	KdPrint(("Leave HelloDDKWrite\n"));

	return status;
}

NTSTATUS HelloDDKQueryInfomation(IN PDEVICE_OBJECT pDevObj, IN PIRP pIrp)
{
	KdPrint(("Enter HelloDDKQueryInfomation\n"));
	NTSTATUS status = STATUS_SUCCESS;

	PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(pIrp);
	PDEVICE_EXTENSION pDevExt = pDevObj->DeviceExtension;
	FILE_INFORMATION_CLASS info = stack->Parameters.QueryFile.FileInformationClass;
	if (info = FileStandardInformation)
	{
		KdPrint(("FileStandardInformation!"));
		PFILE_STANDARD_INFORMATION file_info = (PFILE_STANDARD_INFORMATION)pIrp->AssociatedIrp.SystemBuffer;
		file_info->EndOfFile = RtlConvertLongToLargeInteger(pDevExt->uFileLen);
	}
	pIrp->IoStatus.Status = status;
	pIrp->IoStatus.Information = stack->Parameters.QueryFile.Length;
	KdPrint(("Leave HelloDDKQueryInfomation\n"));
	return status;
}