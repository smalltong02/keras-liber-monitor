// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "commutils.h"
#include "utils.h"

using namespace cchips;

BOOL g_initSuccess = FALSE;

BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    {
        BreakPoint;
        g_is_dll_module = true;
        std::shared_ptr<CHipsCfgObject> hipsCfgObject = InitializeConfig();
        if (hipsCfgObject != nullptr && InitializeHook(hipsCfgObject))
        {
            g_initSuccess = TRUE;
        }
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

