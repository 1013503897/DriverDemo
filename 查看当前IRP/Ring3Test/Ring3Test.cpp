#include <Windows.h>
#include <iostream>
#include <tchar.h>

int main()
{
	HANDLE hDevice =
		CreateFile(_T("\\\\.\\HelloDDK"),
			GENERIC_READ | GENERIC_WRITE,
			0,		
			NULL,	
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL);		

	if (hDevice == INVALID_HANDLE_VALUE)
	{
		printf("Failed to obtain file handle to device: "
			"%s with Win32 error code: %d\n",
			"MyWDMDevice", GetLastError());
		return 1;
	}

	CloseHandle(hDevice);
	return 0;
}


