// CorpusExtractor.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <windows.h>
#include <iostream>
#include "CorpusExtractorLib\TextCorpusManager.h"
#include "StaticPEManager\StaticFileManager.h"

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
            "corpus extract toolkit v 1.0c\n\n"
            "Usage: %S <options..>\n"
            "Options:\n"
            "  --input                     input path\n"
            "  [--extract]                 conducting corpus extraction\n"
            "  [--modelcorpus] [--output]  generating model corpus.\n"
            "  [--embedding]               word embedding\n"
            "  [--output]                  output path\n",
            argv[0]
        );
        return -1;
    }

    bool bextract = false;
    bool bembedding = false;
    std::wstring modelw;
    std::wstring input_pathw;
    std::wstring output_pathw;
    std::wstring modelcorpusw;
    std::wstring lablew;
    for (int idx = 1; idx < argc; idx++) {
        if (wcscmp(argv[idx], L"--input") == 0) {
            input_pathw = argv[++idx];
            continue;
        }
        if (wcscmp(argv[idx], L"--output") == 0) {
            output_pathw = argv[++idx];
            continue;
        }
        if (wcscmp(argv[idx], L"--modelcorpus") == 0) {
            modelcorpusw = argv[++idx];
            continue;
        }
        if (wcscmp(argv[idx], L"--label") == 0) {
            lablew = argv[++idx];
            continue;
        }
        if (wcscmp(argv[idx], L"--extract") == 0) {
            bextract = true;
            continue;
        }
        if (wcscmp(argv[idx], L"--embedding") == 0) {
            bembedding = true;
            continue;
        }
        info("Found unsupported argument: %S\n", argv[idx]);
        return -1;
    }

    if (!input_pathw.length()) {
        info("error argument.\n");
        return -1;
    }
    if (modelcorpusw.length() && !output_pathw.length()) {
        info("error argument.\n");
        return -1;
    }
    if (bextract == false && bembedding == false && !modelcorpusw.length()) {
        info("error argument.\n");
        return -1;
    }

    auto& manager = cchips::GetCTextCorpusManager().GetInstance();
    std::string input_path = to_byte_string(input_pathw);
    manager.Initialize(input_path);
    if (bextract) {
        manager.CorpusExtract();
    }
    if (bembedding) {
        manager.WordEmbedding();
    }
    if (modelcorpusw.length() && output_pathw.length()) {
        std::string output_path = to_byte_string(output_pathw);
        std::string modelcorpus = to_byte_string(modelcorpusw);
        std::string lable = to_byte_string(lablew);
        manager.GeneratingModelDatasets(modelcorpus, output_path, lable);
    }
    return 0;
}
