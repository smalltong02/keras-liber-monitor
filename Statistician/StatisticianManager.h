#pragma once
#include <windows.h>
#include <filesystem>
#include <fstream>
#include "..\LogObject.h"
#include "StaticPeManager/TraverseDriveEngine.h"

namespace cchips {
    namespace fs = std::filesystem;

    class CStatisticianManager {
    public:
        static CStatisticianManager& GetInstance()
        {
            static CStatisticianManager m_instance;
            return m_instance;
        }

        bool Scan(const std::string& scan_path, std::string& output, bool boutput_console = false) {
            m_boutput_console = boutput_console;
            if (!fs::exists(scan_path)) {
                error_log("scan_path %s is not exist!", scan_path.c_str());
                return false;
            }
            if (fs::is_directory(scan_path)) {
                if (OutputConsoleFlag()) {
                    std::cout << "scan folder: " << scan_path << std::endl;
                }
                CTraverseDriveEngine::fi_callback callback = std::bind(&CStatisticianManager::scan_callback, this, std::placeholders::_1, std::placeholders::_2);
                CTraverseDriveEngine::GetInstance().AddFiCb(callback);
                bool bret = CTraverseDriveEngine::GetInstance().TraverseDrive(scan_path, output);
                if (OutputConsoleFlag()) {
                    auto totals = CTraverseDriveEngine::GetInstance().GetFileTotals();
                    std::cout << "scan total files: " << totals << std::endl;
                }
                return bret;
            }
            else if (fs::is_regular_file(scan_path)) {
                return scan_callback(scan_path, output);
            }
            return false;
        }

    private:
        CStatisticianManager() = default;
        ~CStatisticianManager() = default;

        bool OutputConsoleFlag() { return m_boutput_console; }
        bool scan_callback(const std::string& scan_path, std::string& output) {
            auto count_words_in_line = [&](const std::string& input) -> std::uint32_t {
                if (input.empty()) {
                    return 0;
                }
                std::uint32_t word_nums = 0;
                std::istringstream iss(input);
                std::string word;
                while (iss >> word) {
                    word_nums++;
                }
                return word_nums - 1;
            };
            if (OutputConsoleFlag()) {
                std::cout << "scan file: " << scan_path << std::endl;
            }
            std::ifstream infile(scan_path);
            if (!infile.is_open()) {
                error_log("can't open file: %s", scan_path.c_str());
                return false;
            }
            std::string line;
            std::uint32_t line_number = 0;

            std::pair<std::uint32_t, std::uint32_t> minimum_features = {0, 4294967295};
            std::pair<std::uint32_t, std::uint32_t> most_features = { 0, 0 };
            while (std::getline(infile, line)) {
                line_number++;
                std::uint32_t words = count_words_in_line(line);
                if (OutputConsoleFlag()) {
                    std::cout << "the line [" << line_number << "] include [" << words << "] words." << std::endl;
                }
                if (words < minimum_features.second) {
                    minimum_features.first = line_number;
                    minimum_features.second = words;
                }
                if (words > most_features.second) {
                    most_features.first = line_number;
                    most_features.second = words;
                }
            }
            std::cout << std::endl << "minimum features: " << "the line [" << minimum_features.first << "] include [" << minimum_features.second << "] words." << std::endl;
            std::cout << "most features: " << "the line [" << most_features.first << "] include [" << most_features.second << "] words." << std::endl;
            return true;
        }

        std::uint32_t m_boutput_console = false;
    };

#define GetCStatisticianManager() CStatisticianManager::GetInstance()
} // namespace cchips
