// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "commutils.h"
#include "utils.h"
#include "LogObject.h"
#include "HookImplementObject.h"

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
        // please don't block the dllmain, We can not do much more here, because it is risky.
        g_is_dll_module = true;
        std::shared_ptr<CHipsCfgObject> hipsCfgObject = InitializeConfig();
        if (hipsCfgObject != nullptr)
            g_initSuccess = InitializeHook(std::move(hipsCfgObject));
    }
    break;
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
        if (g_initSuccess && g_impl_object)
            g_impl_object->AddFilterThread(std::this_thread::get_id());
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

