#pragma once
#include <windows.h>
#include <winsvc.h>

namespace cchips {

#define MAX_NAME_LEN 64
#define HIPS_DRIVER_NAME "HipsMonitor"

// for kernel dll inject service, set target pid to kernel.
#define IOCTL_HIPS_SETTARGETPID         (ULONG) CTL_CODE(FILE_DEVICE_FIPS,\
    0x809, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_HIPS_SETTARGETCMD         (ULONG) CTL_CODE(FILE_DEVICE_FIPS,\
    0xAA0, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)

    class CDriverMgr
    {
    public:
        CDriverMgr();

        ~CDriverMgr();

        BOOL IoControl(
            DWORD dwIoControlCode,
            LPVOID lpInBuffer,
            DWORD nInBufferSize,
            LPVOID lpOutBuffer,
            DWORD nOutBufferSize,
            LPDWORD lpBytesReturned = NULL
        );

        HANDLE GetHandle() { return m_hdevice; }
        DWORD  GetError() { return m_error; }

    private:
        HANDLE m_hdevice;
        std::string  m_drivername;
        DWORD  m_error;

        BOOL OpenDevice();
    };

} // namespace cchips
