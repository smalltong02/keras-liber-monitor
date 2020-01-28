#include "stdafx.h"
#include "MinHook.h"
#include "commutils.h"
#include "hookimpl.h"
#include "DriverMgr.h"
#include "HipsConfigObject.h"
#include "HookImplementObject.h"
#include "OleConfigObject.h"
#include "LogObject.h"
#include "utils.h"
#include "resource.h"

extern BOOL g_initSuccess;
HMODULE g_orgDll = nullptr;
extern "C" {
	extern FARPROC p[547];
}

CDriverMgr* g_driverMgr = nullptr;
COleConfig* g_oleConfig = nullptr;
CHipsConfigObject* g_hipsConfigObject = nullptr;
CHookImplementObject* g_impl_object = nullptr;
CLogObject* g_log_object = nullptr;

bool InitializeConfig()
{
	//TODO: read hook config information from json file.
	HMODULE ModuleHandle = NULL;
	std::vector<BYTE> ResBuffer;
	ModuleHandle = GetModuleHandle(L"hipshook.dll");

	bool result = ExtractResource(ModuleHandle,
		L"JSONRES",
		MAKEINTRESOURCE(IDR_HIPSHOOK),
		ResBuffer);
	if (result && ResBuffer.size() > 0)
	{
		g_hipsConfigObject = new CHipsConfigObject();
		if (g_hipsConfigObject && g_hipsConfigObject->Initialize(std::string((char*)&ResBuffer[0], ResBuffer.size())))
		{
			return true;
		}
	}
	return false;
}

//bool test_initializeHook()
//{
//	CHookImplementObject impl_object;
//	if (impl_object.test_Initialize())
//	{
//		impl_object.test_HookApi();
//	}
//	return true;
//}

bool InitializeHook()
{
	bool bSuccess = false;
	g_impl_object = new CHookImplementObject();
	g_log_object = new CLogObject();

	if (!g_impl_object || !g_log_object)
		return false;

	if (g_impl_object->Initialize(g_hipsConfigObject) && g_log_object->Initialize() /*&& g_impl_object->HookAllApi()*/)
	{
		bSuccess = true;
	}
	return bSuccess;
}

void UninitialHook()
{
	MH_DisableHook(MH_ALL_HOOKS);
	MH_Uninitialize();
}