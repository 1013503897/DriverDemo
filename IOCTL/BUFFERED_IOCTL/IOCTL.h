#include <ntifs.h>
#include "Ioctls.h"

typedef struct _DEVICE_EXTENSION
{
	PDEVICE_OBJECT pDevObj;
	UNICODE_STRING usDevName;
	UNICODE_STRING usSymbolName;
	PUCHAR buffer;
	ULONG file_length;
}DEVICE_EXTENSION,*PDEVICE_EXTENSION;

#define arraysize(p) (sizeof(p)/sizeof((p)[0]))
#define MAX_FILE_LENGTH 1024

VOID HelloDDKUnload(IN PDRIVER_OBJECT pDriverObject);
NTSTATUS CreateDevice(IN PDRIVER_OBJECT pDriverObject);
NTSTATUS HelloDDKDispatchRoutin(IN PDEVICE_OBJECT pDevObj, IN PIRP pIrp);
NTSTATUS HelloDDKDeviceIOControl(IN PDEVICE_OBJECT pDevObj, IN PIRP pIrp);
