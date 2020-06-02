#include "stdafx.h"
#include "..\InjectImpl.h"

void error(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
}

int main()
{
    LPWSTR *argv; int argc;

    argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (argv == NULL) {
        error("Error parsing commandline options!\n");
        return -1;
    }

    if (argc != 7) {
        error(
            "Usage: %S <options..>\n"
            "Options:\n"
            "  --pid                     be injected process.\n"
            "  --tid                     be injected thread.\n"
            "  --dll                     inject dll path\n",
            argv[0]
        );
        return -1;
    }

    DWORD_PTR pid = 0;
    DWORD_PTR tid = 0;
    std::wstring dll_path;
    for (int idx = 1; idx < argc; idx++) {
        if (wcscmp(argv[idx], L"--pid") == 0) {
            pid = wcstol(argv[++idx], NULL, 10);
            continue;
        }

        if (wcscmp(argv[idx], L"--tid") == 0) {
            tid = wcstol(argv[++idx], NULL, 10);
            continue;
        }

        if (wcscmp(argv[idx], L"--dll") == 0) {
            dll_path = argv[++idx];
            continue;
        }
        error("Found unsupported argument: %S\n", argv[idx]);
        return -1;
    }

    if (pid == 0 || tid == 0 || !dll_path.length())
    {
        error("Found error argument!\n");
        return -1;
    }

    std::unique_ptr<cchips::CDllInjectorObject> injector_object = std::make_unique<cchips::CDllInjectorObject>();
    if (!injector_object)
    {
        error("CDllInjectorObject Class: injector_object create failed!\n");
        return -1;
    }

    std::string narrow_name = cchips::special_log::to_byte_string(dll_path);
    if (!narrow_name.length()) return false;
#ifdef _X86_
    std::unique_ptr<cchips::CDllInjectorObject::_InjectDllInfo> p_dll_info = nullptr;
    if(cchips::CInjectProcess::Is64BitOS())
        p_dll_info = std::make_unique<cchips::CDllInjectorObject::_InjectDllInfo>(cchips::CDllInjectorObject::inject_apc, cchips::CDllInjectorObject::dll_type_wow64, narrow_name);
    else
        p_dll_info = std::make_unique<cchips::CDllInjectorObject::_InjectDllInfo>(cchips::CDllInjectorObject::inject_apc, cchips::CDllInjectorObject::dll_type_x32, narrow_name);
#endif
#ifdef _AMD64_
    std::unique_ptr<cchips::CDllInjectorObject::_InjectDllInfo> p_dll_info = std::make_unique<cchips::CDllInjectorObject::_InjectDllInfo>(cchips::CDllInjectorObject::inject_apc, cchips::CDllInjectorObject::dll_type_x64, narrow_name);
#endif
    if (!p_dll_info)
    {
        error("CDllInjectorObject Class: create p_dll_info failed!\n");
        return -1;
    }
    injector_object->AddDllInfo(p_dll_info);
    if (!injector_object->InjectProcess(pid))
    {
        error("CDllInjectorObject Class: inject failed!\n");
        return -1;
    }
    error("CDllInjectorObject Class: inject success!\n");
    return 0;
}
