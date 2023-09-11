// Collector.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "resource.h"
#include <iostream>
#include "StaticPEManager\StaticFileManager.h"

void info(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
}

#define default_data_path "\\data"

int main()
{
    LPWSTR* argv; int argc;

    argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (argv == NULL) {
        info("Error parsing commandline options!\n");
        return -1;
    }

    if (argc < 3) {
        info(
            "feature collection toolkit v 1.0c\n\n"
            "Usage: %S <options..>\n"
            "Options:\n"
            "  --input                    scan path\n"
            "  [--package]               conducting package extraction\n"
            "  [--cleaning]              data cleaning\n"
            "  [--output]                output path\n",
            argv[0]
        );
        return -1;
    }

    bool bcleaning = false;
    bool bpackage = false;
    std::wstring scan_pathw;
    std::wstring output_pathw;
    std::wstring passwordw;
    for (int idx = 1; idx < argc; idx++) {
        if (wcscmp(argv[idx], L"--input") == 0) {
            scan_pathw = argv[++idx];
            continue;
        }
        if (wcscmp(argv[idx], L"--output") == 0) {
            output_pathw = argv[++idx];
            continue;
        }
        if (wcscmp(argv[idx], L"--package") == 0) {
            bpackage = true;
            continue;
        }
        if (wcscmp(argv[idx], L"--password") == 0) {
            passwordw = argv[++idx];
            continue;
        }
        if (wcscmp(argv[idx], L"--cleaning") == 0) {
            bcleaning = true;
            continue;
        }
        info("Found unsupported argument: %S\n", argv[idx]);
        return -1;
    }
    if (!scan_pathw.length()) {
        info("error argument.\n");
        return -1;
    }

    std::string scan_path = to_byte_string(scan_pathw);
    std::string output_path;
    if (output_pathw.length()) {
        output_path = to_byte_string(output_pathw);
    }
    else {
        std::filesystem::path path = std::filesystem::current_path();
        output_path = path.string() + default_data_path;
    }

    std::unique_ptr<cchips::CJsonOptions> options = std::make_unique<cchips::CJsonOptions>("CFGRES", IDR_JSONPE_CFG);
    if (!options || !options->Parse()) {
        info("load config error.\n");
        return -1;
    }
    auto& manager = cchips::GetCStaticFileManager();
    if (!manager.Initialize(std::move(options))) {
        info("Initialize failed!\n");
        return -1;
    }
    if (bpackage) {
        manager.EnablePackage();
    }
    if (bcleaning) {
        manager.EnableCleaning();
    }
    if (!passwordw.empty()) {
        manager.SetDefaultPwd(passwordw);
    }
    if (!manager.Scan(scan_path, output_path, true)) {
        info("scan failed!\n");
        return -1;
    }
    info("%s\n", manager.GetSuccessScanCount().c_str());
    return 0;
}
