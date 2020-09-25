#include <ntifs.h>

#define MAX_FILE_LENGTH 1024
#define arraysize(p) (sizeof(p)/sizeof((p)[0]))

typedef struct _DEVICE_EXTENSION {
	PDEVICE_OBJECT pDeviceObj;
	UNICODE_STRING usDeviceName;
	UNICODE_STRING usSymbolName;
	PUCHAR pBuffer;
	ULONG uFileLen;
}DEVICE_EXTENSION, * PDEVICE_EXTENSION;


NTSTATUS CreateDevice(IN PDRIVER_OBJECT pDriverObject);
VOID HelloDDKUnload(IN PDRIVER_OBJECT pDriverObject);
NTSTATUS HelloDDKDispatchRoutin(IN PDEVICE_OBJECT pDevObj, IN PIRP pIrp);
NTSTATUS HelloDDKRead(IN PDEVICE_OBJECT pDevObj, IN PIRP pIrp);
NTSTATUS HelloDDKWrite(IN PDEVICE_OBJECT pDevObj, IN PIRP pIrp);
NTSTATUS HelloDDKQueryInfomation(IN PDEVICE_OBJECT pDevObj, IN PIRP pIrp);