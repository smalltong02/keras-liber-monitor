// Statistician.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <windows.h>
#include <iostream>
#include "StatisticianManager.h"

void info(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
}

int main()
{
    LPWSTR* argv; int argc;

    argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (argv == NULL) {
        info("Error parsing commandline options!\n");
        return -1;
    }

    if (argc <= 1) {
        info(
            "statistician toolkit v 1.0c\n\n"
            "Usage: %S <options..>\n"
            "Options:\n"
            "  --input                     input path.\n"
            "  [--mode]                    statistic mode.\n",
            argv[0]
        );
        return -1;
    }

    std::wstring modelw;
    std::wstring input_pathw;
    for (int idx = 1; idx < argc; idx++) {
        if (wcscmp(argv[idx], L"--input") == 0) {
            input_pathw = argv[++idx];
            continue;
        }
        if (wcscmp(argv[idx], L"--mode") == 0) {
            modelw = argv[++idx];
            continue;
        }
    }
    if (!input_pathw.length()) {
        info("error argument.\n");
        return -1;
    }
    auto& manager = cchips::GetCStatisticianManager().GetInstance();
    std::string input_path = to_byte_string(input_pathw);
    std::string output_path;
    manager.Scan(input_path, output_path, true);
    return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
