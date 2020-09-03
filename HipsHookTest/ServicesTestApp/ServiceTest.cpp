#include <stdio.h>
#include <windows.h>
#include <string>
#include <vector>
#include <thread>
#include "..\ServicesBase.h"

using namespace std::chrono_literals;

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
    }

    if (argc != 2) {
        error(
            "Usage: %S <options..>\n"
            "Options:\n"
            "  --services                create a service app for gtest\n"
            "  --normal                  create a normal app for gtest\n",
            "  --loaddll                 Load Hipshook.dll for test\n",
            argv[0]
        );
    }

    bool bservices = false;
    bool bnormal = false;
    bool loaddll = false;
    for (int idx = 1; idx < argc; idx++) {
        if (wcscmp(argv[idx], L"--services") == 0) {
            bservices = true;
            break;
        }

        if (wcscmp(argv[idx], L"--normal") == 0) {
            bnormal = true;
            break;
        }

        if(wcscmp(argv[idx], L"--loaddll") == 0) {
            loaddll = true;
            break;
        }
        error("Found unsupported argument: %S\n", argv[idx]);
        return -1;
    }

    if (bservices)
    {
        ServicesTestObject service;
        service.Run();
        return 0;
    }
    if (bnormal)
    {
        // call LoadLibraryA for generate a R3 log.
        HMODULE  hmodule = LoadLibraryA("user32.dll");
        if (hmodule == nullptr)
            error("LoadLibraryA [user32.dll] failed!\n");
        else
            error("LoadLibraryA [user32.dll] success!\n");
        std::this_thread::sleep_for(5s);
        return 0;
    }
    if (loaddll)
    {
        HMODULE  hmodule = LoadLibraryA("hipshook.dll");
        if (hmodule == nullptr)
            error("LoadLibraryA [user32.dll] failed!\n");
        else
            error("LoadLibraryA [user32.dll] success!\n");
        std::this_thread::sleep_for(10s);
        return 0;
    }
    return -1;
}

