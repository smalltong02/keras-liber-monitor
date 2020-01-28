#pragma once
#include <windows.h>
#include <winsvc.h>

#define MAX_NAME_LEN 64

class CDriverMgr
{
public:
	CDriverMgr(LPCTSTR DriverName);

	~CDriverMgr();

	BOOL IoControl(
		DWORD dwIoControlCode,
		LPVOID lpInBuffer,
		DWORD nInBufferSize,
		LPVOID lpOutBuffer,
		DWORD nOutBufferSize,
		LPDWORD lpBytesReturned = NULL
	);

	HANDLE GetHandle() { return m_hDevice; }
	DWORD  GetError() { return m_dwError; }

protected:
	HANDLE m_hDevice;
	TCHAR  m_szDriverName[MAX_NAME_LEN];
	DWORD  m_dwError;

	BOOL OpenDevice();
};
