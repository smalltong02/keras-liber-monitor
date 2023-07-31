// AppClassifierModel.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <windows.h>
#include <iostream>
#include <codecvt>
#include "Classifier.h"

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
            "model training and predict toolkit v 1.0c\n\n"
            "Usage: %S <options..>\n"
            "Options:\n"
            "  --input                                 dataset or predicting the path of sample.\n"
            "  --model [fasttext | gru | transformers] selecting ML Models.\n"
            "  --model_path                            the path of modelbin.\n"
            "  [--ratio] [8 | 6]                       training ratio.\n"
            "  [--predict]                             predicting the sample using the chosen model.\n"
            "  [--test]                                test the samples using the chosen model.\n"
            "  [--output]                              output to logfile.\n",
            argv[0]
        );
        return -1;
    }

    bool bpredict = false;
    bool btest = false;
    std::wstring modelw;
    std::wstring modelbin_pathw;
    std::wstring dictbin_pathw;
    std::wstring input_pathw;
    std::wstring output_pathw;
    std::wstring ratiow;
    std::wstring kw;
    for (int idx = 1; idx < argc; idx++) {
        if (wcscmp(argv[idx], L"--input") == 0) {
            input_pathw = argv[++idx];
            continue;
        }
        if (wcscmp(argv[idx], L"--ratio") == 0) {
            ratiow = argv[++idx];
            continue;
        }
        if (wcscmp(argv[idx], L"--model") == 0) {
            modelw = argv[++idx];
            continue;
        }
        if (wcscmp(argv[idx], L"--model_path") == 0) {
            modelbin_pathw = argv[++idx];
            continue;
        }
        if (wcscmp(argv[idx], L"--dict_path") == 0) {
            dictbin_pathw = argv[++idx];
            continue;
        }
        if (wcscmp(argv[idx], L"--predict") == 0) {
            bpredict = true;
            continue;
        }
        if (wcscmp(argv[idx], L"--test") == 0) {
            btest = true;
            continue;
        }
        if (wcscmp(argv[idx], L"--output") == 0) {
            output_pathw = argv[++idx];
            continue;
        }
        if (wcscmp(argv[idx], L"--k") == 0) {
            kw = argv[++idx];
            continue;
        }
        info("Found unsupported argument: %S\n", argv[idx]);
        return -1;
    }

    if (!input_pathw.length() || !modelw.length()) {
        info("error argument.\n");
        return -1;
    }
    if (bpredict && ratiow.length()) {
        info("error argument.\n");
        return -1;
    }
    if (bpredict && !modelbin_pathw.length()) {
        info("error argument.\n");
        return -1;
    }
    if (bpredict && btest) {
        info("error argument.\n");
        return -1;
    }

    if (!bpredict && !btest && (!ratiow.length() || !output_pathw.length())) {
        info("error argument.\n");
        return -1;
    }

    std::uint32_t ratio = 8;
    if (ratiow.length()) {
        try {
            ratio = std::stoul(ratiow);
        }
        catch (const std::exception& e) {
            ratio = 8;
        }
    }
    std::uint32_t k = 1;
    if (kw.length()) {
        try {
            k = std::stoul(kw);
        }
        catch (const std::exception& e) {
            k = 1;
        }
    }
    std::string input_path = to_byte_string(input_pathw);
    std::string output_path = to_byte_string(output_pathw);
    std::string model = to_byte_string(modelw);
    std::string modelbin_path = to_byte_string(modelbin_pathw);
    std::string dictbin_path;
    if (dictbin_pathw.length()) {
        dictbin_path = to_byte_string(dictbin_pathw);
    }

    if (ratio != 8 && ratio != 6) {
        ratio = 8;
    }

    if (_stricmp(model.c_str(), "fasttext") == 0) {
        std::unique_ptr<cchips::CFastTextModel> fasttext_model = std::make_unique<cchips::CFastTextModel>(input_path, output_path, modelbin_path, ratio);
        if (!fasttext_model) {
            return false;
        }
        if (bpredict) {
            fasttext_model->SetkParam(k);
            return fasttext_model->predict();
        }
        return fasttext_model->train();
    }
    else if (_stricmp(model.c_str(), "gru") == 0) {
        cchips::CGruModel gru_model(input_path, output_path, modelbin_path, dictbin_path, ratio, false);
        if (!gru_model) {
            return false;
        }
        if (btest) {
            torch::load(gru_model, modelbin_path);
            return gru_model->test();
        }
        if (bpredict) {
            return gru_model->predict();
        }
        if (gru_model->train()) {
            torch::save(gru_model, output_path);
        }
    }
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