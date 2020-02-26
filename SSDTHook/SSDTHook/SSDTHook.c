#include <ntifs.h>

#define PROCESS_TERMINATE         0x0001

typedef struct _KSYSTEM_SERVICE_TABLE
{
	PULONG ServiceTableBase;
	PULONG ServiceCounterTableBase;
	ULONG  NumberOfService;
	ULONG ParamTableBase;
}KSYSTEM_SERVICE_TABLE;

typedef struct _KSERVICE_TABLE_DESCRIPTOR
{
	KSYSTEM_SERVICE_TABLE ntoskrnl; //ntoskrnl.exe SSDT
	KSYSTEM_SERVICE_TABLE win32k;   //win32k.sys ShadowSSDT
	KSYSTEM_SERVICE_TABLE notUsed1;
	KSYSTEM_SERVICE_TABLE notUsed2;
}KSERVICE_TABLE_DESCRIPTOR;

typedef NTSTATUS(NTAPI* FuZwOpenProcess)(
	_Out_ PHANDLE ProcessHandle,
	_In_ ACCESS_MASK DesiredAccess,
	_In_ POBJECT_ATTRIBUTES ObjectAttributes,
	_In_opt_ PCLIENT_ID ClientId
	);

NTSTATUS NTAPI MyZwOpenProcess(
	_Out_ PHANDLE ProcessHandle,
	_In_ ACCESS_MASK DesiredAccess,
	_In_ POBJECT_ATTRIBUTES ObjectAttributes,
	_In_opt_ PCLIENT_ID ClientId
);


FuZwOpenProcess g_OldZwOpenProcess;
KSERVICE_TABLE_DESCRIPTOR* g_pServiceTable = NULL;
ULONG g_Pid = 3456;


void InstallHook();
void UninstallHook();
void ShutPageProtect();
void OpenPageProtect();
void OutLoad(DRIVER_OBJECT* obj);


NTSTATUS DriverEntry(DRIVER_OBJECT* driver, UNICODE_STRING* path)
{
	KdPrint(("驱动启动成功！\n"));
	KdBreakPoint();
	//DbgBreakPoint();
	InstallHook();

	driver->DriverUnload = OutLoad;
	return STATUS_SUCCESS;
}
void OutLoad(DRIVER_OBJECT* obj)
{
	UninstallHook();
}

void InstallHook()
{
	PETHREAD pNowThread = PsGetCurrentThread();
	g_pServiceTable = (KSERVICE_TABLE_DESCRIPTOR*)
		(*(ULONG*)((ULONG)pNowThread + 0xbc));
	g_OldZwOpenProcess = (FuZwOpenProcess)
		g_pServiceTable->ntoskrnl.ServiceTableBase[0xbe];
	ShutPageProtect();
	g_pServiceTable->ntoskrnl.ServiceTableBase[0xbe]
		= (ULONG)MyZwOpenProcess;
	OpenPageProtect();
}

void UninstallHook()
{
	ShutPageProtect();
	g_pServiceTable->ntoskrnl.ServiceTableBase[0xbe]
		= (ULONG)g_OldZwOpenProcess;
	OpenPageProtect();
}

void _declspec(naked) ShutPageProtect()
{
	__asm
	{
		push eax;
		mov eax, cr0;
		and eax, ~0x10000;
		mov cr0, eax;
		pop eax;
		ret;
	}
}

void _declspec(naked) OpenPageProtect()
{
	__asm
	{
		push eax;
		mov eax, cr0;
		or eax, 0x10000;
		mov cr0, eax;
		pop eax;
		ret;
	}
}

NTSTATUS NTAPI MyZwOpenProcess(
	_Out_ PHANDLE ProcessHandle,
	_In_ ACCESS_MASK DesiredAccess,
	_In_ POBJECT_ATTRIBUTES ObjectAttributes,
	_In_opt_ PCLIENT_ID ClientId
)
{
	if (ClientId->UniqueProcess == (HANDLE)g_Pid &&
		DesiredAccess == PROCESS_TERMINATE)
	{
		DesiredAccess = 0;
	}
	return g_OldZwOpenProcess(
		ProcessHandle,
		DesiredAccess,
		ObjectAttributes,
		ClientId);
}