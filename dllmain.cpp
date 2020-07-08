// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "commutils.h"
#include "utils.h"
#include "LogObject.h"

using namespace cchips;

BOOL g_initSuccess = FALSE;

DWORD WINAPI main_thread(LPVOID lpParameter)
{
    BreakPoint;
    std::shared_ptr<CHipsCfgObject> hipsCfgObject = InitializeConfig();
    std::string result_string;
    if (hipsCfgObject != nullptr && InitializeHook(hipsCfgObject))
        result_string = "Hipshook initialize success!";
    else
        result_string = "Hipshook initialize failed!";
    //static std::once_flag flag;
    //std::call_once(flag, [](const std::string log_string) { 
    //    while (1)
    //    {
    //        std::this_thread::sleep_for(std::chrono::seconds(10));
    //        error_log(log_string.c_str());
    //    }
    //    }, result_string);
    return 0;
}

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
        HANDLE thread_handle = CreateThread(NULL, 0, main_thread, NULL, 0, NULL);
        CloseHandle(thread_handle);
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

