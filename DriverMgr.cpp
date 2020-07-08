#include "stdafx.h"
#include "DriverMgr.h"
#include <Setupapi.h>
#include <TCHAR.H>

#pragma comment( lib, "setupapi.lib" ) 
namespace cchips {

    CDriverMgr::CDriverMgr()
        :m_hdevice(INVALID_HANDLE_VALUE)
        , m_error(0)
    {
        m_drivername = HIPS_DRIVER_NAME;
        OpenDevice();
    }

    CDriverMgr::~CDriverMgr()
    {
        if (m_hdevice != INVALID_HANDLE_VALUE)
            CloseHandle(m_hdevice);
    }

    BOOL CDriverMgr::OpenDevice()
    {
        std::string    device_name;
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
        device_name = std::string("\\\\.\\") + m_drivername;

        m_hdevice = CreateFile(device_name.c_str(),
            GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL
        );
        if (m_hdevice == INVALID_HANDLE_VALUE || m_hdevice == nullptr)
        {
            m_error = GetLastError();
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
            m_hdevice,
            dwIoControlCode,
            lpInBuffer,
            nInBufferSize,
            lpOutBuffer,
            nOutBufferSize,
            &dwReturned,
            NULL);

        if (lpBytesReturned)
            *lpBytesReturned = dwReturned;

        m_error = GetLastError();
        return (bRet);
    }

} // namespace cchips
