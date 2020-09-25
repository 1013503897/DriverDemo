#include <ntifs.h>

typedef struct _DEVICE_EXTENSION {
	PDEVICE_OBJECT pDevice;
	UNICODE_STRING ustrDeviceName;	
	UNICODE_STRING ustrSymLinkName;	
} DEVICE_EXTENSION, * PDEVICE_EXTENSION;

#define arraysize(p) (sizeof(p)/sizeof((p)[0]))

VOID HelloDDKUnload(IN PDRIVER_OBJECT pDriverObject);
NTSTATUS DriverEntry(PDRIVER_OBJECT pDriverObject, PUNICODE_STRING pRegistryPath);
NTSTATUS CreateDevice(IN PDRIVER_OBJECT pDriverObject);
NTSTATUS HelloDDKDispatchRoutin(IN PDEVICE_OBJECT pDevObj, IN PIRP pIrp);
