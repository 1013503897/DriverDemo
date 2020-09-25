#include <ntifs.h>

#define arraysize(p) (sizeof(p)/sizeof((p)[0]))

#define MAX_FILE_LENGTH 1024

typedef struct _DEVICE_EXTENSION {
	PDEVICE_OBJECT pDevice;
	UNICODE_STRING usDeviceName;	
	UNICODE_STRING ustrSymLinkName;	

	PUCHAR buffer;
	ULONG file_length;
} DEVICE_EXTENSION, * PDEVICE_EXTENSION;

NTSTATUS CreateDevice(IN PDRIVER_OBJECT pDriverObject);
VOID HelloDDKUnload(IN PDRIVER_OBJECT pDriverObject);
NTSTATUS HelloDDKDispatchRoutin(IN PDEVICE_OBJECT pDevObj,
	IN PIRP pIrp);
NTSTATUS HelloDDKRead(IN PDEVICE_OBJECT pDevObj,
	IN PIRP pIrp);
NTSTATUS HelloDDWrite(IN PDEVICE_OBJECT pDevObj,
	IN PIRP pIrp);
NTSTATUS HelloDDKQueryInfomation(IN PDEVICE_OBJECT pDevObj,
	IN PIRP pIrp);