#include "IOCTL.h"

NTSTATUS DriverEntry(PDRIVER_OBJECT pDriverObject, PUNICODE_STRING RegistryPath)
{
	KdPrint(("Enter DriverEntry\n"));
	NTSTATUS status;

	pDriverObject->DriverUnload = HelloDDKUnload;

	for (int i = 0; i < arraysize(pDriverObject->MajorFunction); ++i)
	{
		pDriverObject->MajorFunction[i] = HelloDDKDispatchRoutin;
	}
	pDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = HelloDDKDeviceIOControl;
	status = CreateDevice(pDriverObject);
	return status;
}

VOID HelloDDKUnload(IN PDRIVER_OBJECT pDriverObject)
{
	PDEVICE_OBJECT pNextDevice;
	KdPrint(("Enter DriverUnload\n"));
	pNextDevice = pDriverObject->DeviceObject;
	while (pNextDevice != NULL)
	{
		PDEVICE_EXTENSION pDevExt = pNextDevice->DeviceExtension;
		IoDeleteSymbolicLink(&pDevExt->usSymbolName);
		if (pDevExt->buffer)
		{
			ExFreePoolWithTag(pDevExt->buffer, "15PB");
			pDevExt->buffer = NULL;
		}
		pNextDevice = pDevExt->pDevObj;
	}
}

NTSTATUS CreateDevice(IN PDRIVER_OBJECT pDriverObject)
{
	NTSTATUS status;
	UNICODE_STRING devName;
	UNICODE_STRING symbolName;
	PDEVICE_EXTENSION pDevExt;
	PDEVICE_OBJECT pDevObj;

	RtlInitUnicodeString(&devName, L"\\Device\\MyDDKDevice");
	RtlInitUnicodeString(&symbolName, L"\\??\\HelloDDK");

	status = IoCreateDevice(pDriverObject,
		sizeof(DEVICE_EXTENSION),
		&devName,
		FILE_DEVICE_UNKNOWN,
		0, TRUE,
		&pDevObj);
	if (!NT_SUCCESS(status))
	{
		return status;
	}
	pDevObj->Flags |= DO_DIRECT_IO;
	pDevExt = pDevObj->DeviceExtension;
	pDevExt->usDevName = devName;
	pDevExt->usSymbolName = symbolName;
	pDevExt->buffer = (PUCHAR)ExAllocatePoolWithTag(PagedPool, MAX_FILE_LENGTH, "15PB");
	pDevExt->file_length = 0;
	status = IoCreateSymbolicLink(&symbolName, &devName);
	if (!NT_SUCCESS(status))
	{
		IoDeleteDevice(pDevObj);
		return status;
	}
	return STATUS_SUCCESS;
}

NTSTATUS HelloDDKDispatchRoutin(IN PDEVICE_OBJECT pDevObj, IN PIRP pIrp)
{
	KdPrint(("Enter HelloDDKDispatchRoutin\n"));

	PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(pIrp);
	//建立一个字符串数组与IRP类型对应起来
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
	// 完成IRP
	pIrp->IoStatus.Status = status;
	pIrp->IoStatus.Information = 0;	// bytes xfered
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);

	KdPrint(("Leave HelloDDKDispatchRoutin\n"));

	return status;
}

NTSTATUS HelloDDKDeviceIOControl(IN PDEVICE_OBJECT pDevObj, IN PIRP pIrp)
{
	NTSTATUS status = STATUS_SUCCESS;
	KdPrint(("Enter HelloDDKDeviceIOControl\n"));

	PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(pIrp);
	ULONG cbin = stack->Parameters.DeviceIoControl.InputBufferLength;
	ULONG cbout = stack->Parameters.DeviceIoControl.OutputBufferLength;
	ULONG code = stack->Parameters.DeviceIoControl.IoControlCode;

	ULONG info = 0;
	switch (code)
	{						
	case IOCTL_TEST1:
	{
		KdPrint(("IOCTL_TEST1\n"));
		UCHAR* InputBuffer = (UCHAR*)pIrp->AssociatedIrp.SystemBuffer;
		for (ULONG i = 0; i < cbin; i++)
		{
			KdPrint(("%X\n", InputBuffer[i]));
		}

		UCHAR* OutputBuffer = (UCHAR*)pIrp->AssociatedIrp.SystemBuffer;
		memset(OutputBuffer, 0xAA, cbout);
		info = cbout;
		break;
	}
	case IOCTL_TEST2:
	{
		KdPrint(("IOCTL_TEST2\n"));
		UCHAR* InputBuffer = (UCHAR*)pIrp->AssociatedIrp.SystemBuffer;
		for (ULONG i = 0; i < cbin; i++)
		{
			KdPrint(("%X\n", InputBuffer[i]));
		}

		KdPrint(("User Address:0X%08X\n", MmGetMdlVirtualAddress(pIrp->MdlAddress)));

		UCHAR* OutputBuffer = (UCHAR*)MmGetSystemAddressForMdlSafe(pIrp->MdlAddress, NormalPagePriority);
		memset(OutputBuffer, 0xAA, cbout);
		info = cbout;
		break;
	}
	case IOCTL_TEST3:
	{
		KdPrint(("IOCTL_TEST3\n"));
		UCHAR* UserInputBuffer = (UCHAR*)stack->Parameters.DeviceIoControl.Type3InputBuffer;
		KdPrint(("UserInputBuffer:0X%0X\n", UserInputBuffer));

		PVOID UserOutputBuffer = pIrp->UserBuffer;

		KdPrint(("UserOutputBuffer:0X%0X\n", UserOutputBuffer));

		__try
		{
			KdPrint(("Enter __try block\n"));

			ProbeForRead(UserInputBuffer, cbin, 4);
			for (ULONG i = 0; i < cbin; i++)
			{
				KdPrint(("%X\n", UserInputBuffer[i]));
			}

			ProbeForWrite(UserOutputBuffer, cbout, 4);

			memset(UserOutputBuffer, 0xAA, cbout);

			//由于在上面引发异常，所以以后语句不会被执行!
			info = cbout;

			KdPrint(("Leave __try block\n"));
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			KdPrint(("Catch the exception\n"));
			KdPrint(("The program will keep going\n"));
			status = STATUS_UNSUCCESSFUL;
		}

		info = cbout;
		break;
	}

	default:
		status = STATUS_INVALID_VARIANT;
	}

	// 完成IRP
	pIrp->IoStatus.Status = status;
	pIrp->IoStatus.Information = info;	
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);

	KdPrint(("Leave HelloDDKDeviceIOControl\n"));

	return status;
}
