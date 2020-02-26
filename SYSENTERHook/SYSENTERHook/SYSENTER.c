#include <ntifs.h>
ULONG_PTR g_OldKiFastCallEntry = 0;
ULONG g_Access = 1;

void InstallHook();
void UninstallHook();
void MyKiFastCallEntry();

void OutLoad(DRIVER_OBJECT* obj)
{
	obj;
	UninstallHook();
}
NTSTATUS DriverEntry(DRIVER_OBJECT* driver, UNICODE_STRING* path)
{
	path;
	KdPrint(("驱动启动成功！\n"));
	KdBreakPoint();
	InstallHook();

	driver->DriverUnload = OutLoad;
	return STATUS_SUCCESS;
}

void __declspec(naked) InstallHook()
{
	__asm
	{
		push eax;
		push ecx;
		push edx;
		mov ecx, 0x176;
		rdmsr;          
		mov[g_OldKiFastCallEntry], eax;

		mov eax, MyKiFastCallEntry;
		xor edx, edx;
		wrmsr;

		pop edx;
		pop ecx;
		pop eax;
		ret;
	}
}

void UninstallHook()
{
	KdBreakPoint();
	__asm
	{
		push eax;
		push ecx;
		push edx;

		mov ecx, 0x176;
		mov eax, [g_OldKiFastCallEntry];
		xor edx, edx;
		wrmsr;

		pop edx;
		pop ecx;
		pop eax;
	}
}

void _declspec(naked) MyKiFastCallEntry()
{
	__asm
	{
		cmp eax, 0xbe;
		jne _End;   

		cmp[edx + 0xc], 1;
		jne _End;
		mov[edx + 0xc], 0;
	_End:
		jmp g_OldKiFastCallEntry;
	}
}