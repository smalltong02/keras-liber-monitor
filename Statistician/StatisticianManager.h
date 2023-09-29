#pragma once
#include <windows.h>
#include <filesystem>
#include <fstream>
#include "..\LogObject.h"
#include "StaticPeManager/TraverseDriveEngine.h"
#include "RapidJsonWrapper.h"

namespace cchips {
    namespace fs = std::filesystem;

    class CStatisticianManager {
    public:
        using statistician_model = enum {
            statistics_unknown,
            statistics_corpus,
            statistics_dupapps,
        };

        static CStatisticianManager& GetInstance()
        {
            static CStatisticianManager m_instance;
            return m_instance;
        }

        bool Scan(const std::string& scan_path, std::string& output, const std::string& model, bool boutput_console = false) {
            auto GetModel = [](const std::string& model) ->statistician_model {
                if (model.empty()) {
                    return statistics_unknown;
                }
                if (model == "corpus") {
                    return statistics_corpus;
                }
                if (model == "dupapps") {
                    return statistics_dupapps;
                }
                return statistics_unknown;
            };
            m_boutput_console = boutput_console;
            m_model = GetModel(model);
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
        void SetPrefix(const std::string& str) {
            m_prefix = str;
        }
        std::string RemovePrefix(const std::string str) {
            if (m_prefix.empty()) {
                return str;
            }
            if (str.empty()) {
                return str;
            }
            auto nfind = str.find(m_prefix);
            if (nfind != 0) {
                return str;
            }
            std::string tmpstr = str.substr(m_prefix.length());
            if (!tmpstr.empty()) {
                if (tmpstr[0] == '\\') {
                    tmpstr = tmpstr.substr(1);
                }
            }
            return tmpstr;
        }

    private:
        CStatisticianManager() = default;
        ~CStatisticianManager() = default;

#define label_prefix_string "__label_"
#define _extract_padding " --> extract: "
        bool GeneratingDupappsDatasets(std::unique_ptr<CRapidJsonWrapper> wrapper, std::vector<std::string>& datasets);

        bool IsLabel(const std::string& str) const {
            auto found = str.find(label_prefix_string);
            if (found == 0) {
                return true;
            }
            return false;
        }
        bool SkipLabel() const { return m_skip_label; }
        bool OutputConsoleFlag() { return m_boutput_console; }
        void IncFeatureCatlist(std::uint32_t words) {
            std::uint32_t pre_words = 0;
            for (auto& item : m_feature_cats_list) {
                if (words > pre_words && words <= item.first) {
                    item.second++;
                    break;
                }
                pre_words = item.first;
            }
            return;
        }
        void OutputFeatureCatlist() {
            std::uint32_t pre_words = 0;
            std::cout << std::endl;
            for (auto& item : m_feature_cats_list) {
                std::cout << "features[" << pre_words << ":" << item.first << "]: " << item.second << std::endl;
                pre_words = item.first;
            }
            return;
        }
        bool scan_callback(const std::string& scan_path, std::string& output) {
            if (scan_path.empty()) {
                return false;
            }
            if (OutputConsoleFlag()) {
                std::cout << "scan file: " << scan_path << std::endl;
            }
            switch (m_model) {
            case statistics_corpus:
            {
                return scan_statistics_corpus(scan_path, output);
            }
            break;
            case statistics_dupapps:
            {
                return scan_statistics_dupapps(scan_path, output);
            }
            break;
            default:
                break;
            }
            return false;
        }
        bool scan_statistics_corpus(const std::string& scan_path, std::string& output) {
            auto count_words_in_line = [&](const std::string& input) -> std::uint32_t {
                if (input.empty()) {
                    return 0;
                }
                std::uint32_t word_nums = 0;
                std::istringstream iss(input);
                std::string word;
                while (iss >> word) {
                    if (SkipLabel()) {
                        if (IsLabel(word)) {
                            continue;
                        }
                    }
                    word_nums++;
                }
                return word_nums - 1;
            };
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
                IncFeatureCatlist(words);
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
            std::cout << std::endl << "totalsets counts: " << line_number << std::endl;
            OutputFeatureCatlist();
            return true;
        }
        bool LoadFile(const std::string& path, std::string& buffer)
        {
            if (path.empty()) {
                return false;
            }
            if (!fs::exists(path)) {
                return false;
            }
            buffer.resize(0);

            auto filesize = fs::file_size(path);
            if (filesize == static_cast<uintmax_t>(-1) || filesize == 0) {
                return false;
            }
            std::ifstream infile;
            infile.open(path, std::ios::in | std::ios::binary | std::ios::ate);
            if (!infile.is_open()) {
                return false;
            }
            infile.seekg(0, std::ios::beg);
            buffer.resize(filesize);
            if (buffer.size() != filesize) {
                buffer.resize(0);
                return false;
            }
            infile.read((char*)&buffer[0], filesize);
            auto readed = infile.tellg();
            if (!readed) {
                buffer.resize(0);
                return false;
            }
            return true;
        }
        bool IsJsonFile(const std::string& buffer) const {
            CRapidJsonWrapper wrapper(buffer);
            if (wrapper.IsValid()) {
                return true;
            }
            return false;
        }
        bool scan_statistics_dupapps(const std::string& scan_path, std::string& output) {
            std::string filebuffer;
            if (output.empty()) {
                return false;
            }
            if (LoadFile(scan_path, filebuffer)) {
                std::unique_ptr<CRapidJsonWrapper> wrapper = std::make_unique<CRapidJsonWrapper>(filebuffer);
                if (!wrapper || !wrapper->IsValid()) {
                    return false;
                }
                std::vector<std::string> datasets;
                bool bret = GeneratingDupappsDatasets(std::move(wrapper), datasets);
                if (bret) {
                    if (datasets.size()) {
                        std::ofstream outfile(output, std::ios::out);
                        if (!outfile.is_open())
                            return false;
                        for (auto& set : datasets) {
                            outfile << set << std::endl;
                        }
                    }
                    std::cout << "dup_words: " << datasets.size() << std::endl;
                    return true;
                }
            }
            return false;
        }

        std::vector<std::pair<std::uint32_t, std::uint32_t>> m_feature_cats_list = {
                {500, 0},
                {1000, 0},
                {1500, 0},
                {2000, 0},
                {2500, 0},
                {3000, 0},
                {3500, 0},
                {4000, 0},
                {4500, 0},
                {5000, 0},
                {5500, 0},
                {4294967295, 0},
        };
        bool m_skip_label = true;
        statistician_model m_model = statistics_unknown;
        std::uint32_t m_boutput_console = false;
        std::string m_prefix;
    };

#define GetCStatisticianManager() CStatisticianManager::GetInstance()
} // namespace cchips
