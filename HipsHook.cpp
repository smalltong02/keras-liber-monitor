// win32_hook.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "HipsHook.h"


// This is an example of an exported variable
WIN32_HOOK_API int nwin32_hook=0;

// This is an example of an exported function.
WIN32_HOOK_API int fnwin32_hook(void)
{
    return 42;
}

// This is the constructor of a class that has been exported.
// see win32_hook.h for the class definition
Cwin32_hook::Cwin32_hook()
{
    return;
}
