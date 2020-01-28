#pragma once
#include "commutils.h"
#include <vector>

#define CHARS_IN_GUID 39
#define MAX_OLE_CHAR 64

enum wmi_ClassId
{
	wmi_Win32_ComputerSystem = 1,
	wmi_Win32_DiskDrive,
	wmi_Win32_Processor,
	wmi_Win32_LogicalDisk,
	wmi_Win32_NetworkAdapterConfiguration,
	wmi_Win32_BaseBoard,
	wmi_Win32_CDROMDrive,
	wmi_Win32_BIOS,
	wmi_Win32_LoggedOnUser,
	wmi_Win32_OperatingSystem,
	wmi_Win32_PhysicalMemory,
	wmi_Win32_PhysicalMemoryArray,
	wmi_Win32_Process,
	wmi_Win32_ProcessStartup,
};

typedef struct tagMULTI_idQI
{
	IID idIID;
	IID queryIID;
	IUnknown *pItf;
	HRESULT hr;
} 	MULTI_idQI;

typedef struct _ComClassGUIDString
{
	GUID iid;
	wchar_t ObjectName[MAX_OLE_CHAR];

} ComClassGUIDString;

typedef struct _WmiClassIdString
{
	wmi_ClassId classId;
	wchar_t ObjectName[MAX_OLE_CHAR];

} WmiClassIdString;

#define WND_UI_NAME L"FortiTracerMonitorUi"

#define IsEqualIIID(riid1, riid2) (!memcmp(riid1, riid2, sizeof(GUID)))

bool StartsWith(wchar_t *heystack, wchar_t *needle, bool case_sensitive);
bool EndsWith(wchar_t *heystack, wchar_t *needle, bool case_sensitive);
bool IsValidGUID(GUID& iid);
GUID* GetQueryIID(const BSTR QueryName);
bool IsCloneComObject(MULTI_idQI& qi);
void CloneComObject(GUID* riid, IID* queryIID, void **ppvObject, void* pThis = NULL);
std::wstring A2W(std::string str);
std::string W2A(std::wstring str);
GUID GetGUIDFromString(std::wstring& Name);
int GetClassIdFromString(std::wstring& Name);
std::wstring GetStringFromGUID(GUID& iid);
void DbgPrintLog(std::wstring notice_string);
void WmiInterfaceQueryLog(int ClassId, std::wstring Properties, VARIANT& Value);
bool ExtractResource(HMODULE ModuleHandle, TCHAR const * ResourceName,
	TCHAR const * ResourceId, std::vector<BYTE>& ResoureBuffer);

#ifdef _AMD64_
extern "C" {
	extern void __stdcall BreakInt3(void);
}
#endif


