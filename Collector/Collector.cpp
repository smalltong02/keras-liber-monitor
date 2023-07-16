// Collector.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "resource.h"
#include <iostream>
#include "StaticPEManager\StaticFileManager.h"

std::string to_byte_stringa(const std::wstring& input)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    return converter.to_bytes(input);
}

std::wstring to_wide_stringa(const std::string& input)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    return converter.from_bytes(input);
}

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

    if (argc < 3) {
        info(
            "Usage: %S <options..>\n"
            "Options:\n"
            "  --path                    scan path\n"
            "  [--output]                output path\n",
            argv[0]
        );
        return -1;
    }

    std::wstring scan_pathw;
    std::wstring output_pathw;
    for (int idx = 1; idx < argc; idx++) {
        if (wcscmp(argv[idx], L"--path") == 0) {
            scan_pathw = argv[++idx];
            continue;
        }
        if (wcscmp(argv[idx], L"--output") == 0) {
            output_pathw = argv[++idx];
            continue;
        }
        info("Found unsupported argument: %S\n", argv[idx]);
        return -1;
    }
    if (!scan_pathw.length()) {
        info("error argument.\n");
        return -1;
    }

    std::string scan_path = to_byte_stringa(scan_pathw);
    std::string output_path;
    if (output_pathw.length()) {
        output_path = to_byte_stringa(output_pathw);
    }
    else {
        std::filesystem::path path = std::filesystem::current_path();
        output_path = path.string();
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
    if (!manager.Scan(scan_path, output_path, true)) {
        info("scan failed!\n");
        return -1;
    }
    info("%s\n", manager.GetSuccessScanCount().c_str());
    return 0;
}
