#include "stdafx.h"
#include "DriverMgr.h"
#include <Setupapi.h>
#include <TCHAR.H>

#pragma comment( lib, "setupapi.lib" ) 

CDriverMgr::CDriverMgr(LPCTSTR DriverName)
	:m_hDevice(INVALID_HANDLE_VALUE)
	, m_dwError(0)
{
	_tcscpy_s(m_szDriverName, MAX_NAME_LEN, DriverName);
	OpenDevice();
}

CDriverMgr::~CDriverMgr()
{
	if (m_hDevice != INVALID_HANDLE_VALUE)
		CloseHandle(m_hDevice);
}

BOOL CDriverMgr::OpenDevice()
{
	TCHAR    completeDeviceName[64];

	//
	// Create a \\.\XXX device name that CreateFile can use
	//
	// NOTE: We're making an assumption here that the driver
	//       has created a symbolic link using it's own name
	//       (i.e. if the driver has the name "XXX" we assume
	//       that it used IoCreateSymbolicLink to create a
	//       symbolic link "\DosDevices\XXX". Usually, there
	//       is this understanding between related apps/drivers.
	//
	//       An application might also peruse the DEVICEMAP
	//       section of the registry, or use the QueryDosDevice
	//       API to enumerate the existing symbolic links in the
	//       system.
	//

	wsprintf(completeDeviceName, TEXT("\\\\.\\%s"), m_szDriverName);

	m_hDevice = CreateFile(completeDeviceName,
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);
	if (m_hDevice == ((HANDLE)-1))
	{
		m_dwError = GetLastError();
		return FALSE;
	}

	return TRUE;
}

BOOL CDriverMgr::IoControl(
	DWORD dwIoControlCode,
	LPVOID lpInBuffer,
	DWORD nInBufferSize,
	LPVOID lpOutBuffer,
	DWORD nOutBufferSize,
	LPDWORD lpBytesReturned
)
{
	BOOL  bRet = 0;
	DWORD dwReturned = 0;

	bRet = DeviceIoControl(
		m_hDevice,
		dwIoControlCode,
		lpInBuffer,
		nInBufferSize,
		lpOutBuffer,
		nOutBufferSize,
		&dwReturned,
		NULL);

	if (lpBytesReturned)
		*lpBytesReturned = dwReturned;

	m_dwError = GetLastError();
	return (bRet);
}

