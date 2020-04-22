#pragma once

#include <stdio.h>
#include <windows.h>
#include <guiddef.h>
#include <WbemCli.h>
#include <WMIUtils.h>
#include <OCIdl.h>
#include <winioctl.h>
#include "HipsCfgObject.h"

using namespace cchips;

// for kernel dll inject service, get config from kernel.
#define IOCTL_HIPS_GET_INJECT32_CONFIG	(ULONG) CTL_CODE(FILE_DEVICE_FIPS,\
	0xA01, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)

// for kernel dll inject service, set cmdline to kernel.
#define IOCTL_HIPS_SETTARGETCMD		(ULONG) CTL_CODE(FILE_DEVICE_FIPS,\
	0xA02, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)

extern GUID CLSID_idWbemDefPath;
extern GUID CLSID_idWbemLocator;
extern GUID IID_idIWbemPath;
extern GUID IID_idIWbemLocator;
extern GUID IID_idIClassFactory;
extern GUID IID_idIClassFactory2;
extern GUID IID_idIManagedObject;
extern GUID IID_idIUnknown;
extern GUID IID_idIWbemCallResult;
extern GUID IID_idIWbemClassObject;
extern GUID IID_idIMarshal;
extern GUID IID_idIWbemServices;
extern GUID IID_idIClientSecurity;
extern GUID IID_idIQueryUnknown;
extern GUID IID_Win32ComputerSystem;
extern GUID IID_Win32DiskDrive;
extern GUID IID_Win32Processor;
extern GUID IID_Win32LogicalDisk;
extern GUID IID_Win32NetworkAdapterConfiguration;
extern GUID IID_Win32BaseBoard;
extern GUID IID_Win32CDROMDrive;
extern GUID IID_Win32BIOS;
extern GUID IID_Win32LoggedOnUser;
extern GUID IID_Win32PhysicalMemory;
extern GUID IID_Win32PhysicalMemoryArray;
extern GUID IID_Win32OperatingSystem;
extern GUID IID_idIEnumWbemClassObject;
extern GUID IID_Win32Process;
extern GUID IID_Win32ProcessStartup;

typedef interface idIUnknown idIUnknown;
typedef interface idIWbemPath idIWbemPath;
typedef interface idIWbemServices idIWbemServices;
typedef interface idIWbemQualifierSet idIWbemQualifierSet;
typedef interface idIWbemCallResult idIWbemCallResult;
typedef interface idIWbemClassObject idIWbemClassObject;
typedef interface idIClientSecurity idIClientSecurity;
typedef interface idWin32IMarshal idWin32IMarshal;
typedef interface idIWbemContext idIWbemContext;
typedef interface idIWbemLocator idIWbemLocator;
typedef interface idIClassFactory idIClassFactory;
typedef interface idIClassFactory2 idIClassFactory2;
typedef interface idIWbemObjectSink idIWbemObjectSink;
typedef interface idIWbemQualifierSet idIWbemQualifierSet;
typedef interface idIEnumWbemClassObject idIEnumWbemClassObject;

std::shared_ptr<CHipsCfgObject> InitializeConfig();
bool InitializeHook(std::shared_ptr<CHipsCfgObject>& hipsConfigObject);
void UninitialHook();
