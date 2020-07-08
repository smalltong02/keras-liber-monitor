#include <stdio.h>
#include <windows.h>
#include <string>
#include <vector>
#include <thread>
#include "..\ServicesBase.h"

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
            argv[0]
        );
    }

    bool bservices = false;
    bool bnormal = false;
    for (int idx = 1; idx < argc; idx++) {
        if (wcscmp(argv[idx], L"--services") == 0) {
            bservices = true;
            break;
        }

        if (wcscmp(argv[idx], L"--normal") == 0) {
            bnormal = true;
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
        std::this_thread::sleep_for(std::chrono::seconds(5));
        return 0;
    }
    return -1;
}

