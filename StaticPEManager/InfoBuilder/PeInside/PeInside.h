#pragma once
#include "..\..\JsonOptions.h"
#include <string>
#include <filesystem>
#include "handler.h"

namespace cchips {
    class CPeInside
    {
    public:
        CPeInside();
        bool Initialize(CJsonOptions::_peinside_info& info);
        ~CPeInside();

        bool Scan(fs::path& path);
        bool ScanPeriod(fs::path& path);
        bool Scan(const std::string& buffer);
        bool ScanPeriod(const std::string& buffer);
        std::unique_ptr<cchips::CRapidJsonWrapper> MoveResult() { return std::move(m_json_result); }
    private:
        std::unique_ptr<PeLib::PeFile> GetPeFile(fs::path& path);
        std::unique_ptr<PeLib::PeFile> GetPeFile(const std::string& buffer);

        std::unique_ptr<cchips::CRapidJsonWrapper> m_json_result;
        std::vector<std::unique_ptr<CPeInsideHandler>> m_handlers;
        CJsonOptions::_peinside_info m_options_info{};
    };
} // namespace cchips
