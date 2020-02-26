#include <windows.h>
#include <stdio.h>
#include <tchar.h>
void main()
{
	HANDLE hDevice = CreateFile(_T("\\\\.\\HelloDDK"),
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
		return;
	}

	DWORD nFileLen = GetFileSize(hDevice, NULL);
	printf("Old File Size %d bytes\n", nFileLen);
	UCHAR pBuffer[10];
	DWORD ulWrite = 0;
	BOOL bRet;
	memset(pBuffer, 0xBB, 10);
	bRet = WriteFile(hDevice, pBuffer, 10, &ulWrite, NULL);
	if (bRet)
	{
		printf("Write %d Bytes\n", ulWrite);
	}
	ULONG ulRead;
	memset(pBuffer, 0, 10);
	bRet = ReadFile(hDevice, pBuffer, 10, &ulRead, NULL);
	if (bRet)
	{
		printf("Read %d bytes:", ulRead);
		for (int i = 0; i < (int)ulRead; i++)
		{
			printf("%02X ", pBuffer[i]);
		}
		printf("\n");
	}
	nFileLen = GetFileSize(hDevice, NULL);
	printf("New File Size %d bytes\n", nFileLen);
	getchar();
	getchar();
	CloseHandle(hDevice);
	return;
}