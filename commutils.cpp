#include "stdafx.h"
#include "MinHook.h"
#include "commutils.h"
#include "hookimpl.h"
#include "DriverMgr.h"
#include "HipsCfgObject.h"
#include "HookImplementObject.h"
#include "LogObject.h"
#include "utils.h"
#include "resource.h"

extern BOOL g_initSuccess;
HMODULE g_orgDll = nullptr;
extern "C" {
	extern FARPROC p[547];
}

CDriverMgr* g_driverMgr = nullptr;
std::shared_ptr<CHipsCfgObject> InitializeConfig()
{
	//TODO: read hook config information from json file.
	HMODULE ModuleHandle = NULL;
	std::vector<BYTE> ResBuffer;
#ifdef WIN32_HOOK_DLL
	ModuleHandle = GetModuleHandle(__TEXT("hipshook.dll"));
#else
	ModuleHandle = GetModuleHandle(NULL);
#endif
	std::shared_ptr<CHipsCfgObject> hipsCfgObject = std::make_shared<CHipsCfgObject>();

	bool result = ExtractResource(ModuleHandle,
		__TEXT("JSONRES"),
		MAKEINTRESOURCE(IDR_HIPSHOOK),
		ResBuffer);
	if (result && ResBuffer.size() > 0)
	{
		if (hipsCfgObject && hipsCfgObject->Initialize(std::string((char*)&ResBuffer[0], ResBuffer.size())))
		{
			return hipsCfgObject;
		}
	}
	return nullptr;
}

bool InitializeHook(std::shared_ptr<CHipsCfgObject>& hipsCfgObject)
{
	bool bSuccess = false;

	if (hipsCfgObject == nullptr || g_impl_object == nullptr || g_log_object == nullptr)
		return false;

	if (g_impl_object->Initialize(hipsCfgObject) && g_log_object->Initialize() && g_impl_object->HookAllApis())
	{
		bSuccess = true;
	}
	return bSuccess;
}

void UninitialHook()
{
	if (g_impl_object)
		g_impl_object->DisableAllApis();
	return;
}