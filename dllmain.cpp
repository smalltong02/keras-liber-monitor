// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "commutils.h"

BOOL g_initSuccess = FALSE;

typedef	void (WINAPI *PDbgBreakPoint)();
PDbgBreakPoint DbgBreakPoint = NULL;


BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		{
//#ifdef _DEBUG
//	#ifdef _AMD64_
//			HMODULE hntdll = LoadLibraryA("ntdll.dll");
//			if (hntdll != NULL)
//			{
//				DbgBreakPoint = (void (WINAPI *)())GetProcAddress(hntdll, "DbgBreakPoint");
//				if(DbgBreakPoint)
//					DbgBreakPoint();
//		}
//	#else
//			__asm int 3;
//	#endif
//#endif

#ifdef _DEBUG
			if (InitializeConfig() && InitializeHook())
			{
				g_initSuccess = TRUE;
			}
#else
			if (InitializeConfig() && InitializeHook())
			{
				g_initSuccess = TRUE;
			}
#endif
		}
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		if (g_initSuccess)
		{
			UninitialHook();
		}
		break;
	}
	return TRUE;
}

