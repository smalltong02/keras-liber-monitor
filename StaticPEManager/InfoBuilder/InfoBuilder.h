#pragma once
#include <chrono>
#include "..\JsonOptions.h"
#include "..\FileInfo.h"
#include "utf8.h"

namespace cchips {

    class CInfoBuilder
    {
    public:

        virtual ~CInfoBuilder() = default;
        virtual bool Initialize(std::unique_ptr<CJsonOptions>& json_options) = 0;

        virtual bool Scan(fs::path& file_path, CFileInfo& file_info) = 0;
        virtual bool ScanPeriod(fs::path& file_path, CFileInfo& file_info) {
            const auto sta = std::chrono::steady_clock::now();
            bool bret = Scan(file_path, file_info);
            m_scan_period = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - sta);
            return bret;
        }
        virtual std::chrono::microseconds GetScanPeriod() const { return m_scan_period; }
        void UpdateInfoType(CJsonOptions::_info_type type) { m_info_type = type; }
        CJsonOptions::_info_type GetInfoType() const { return m_info_type; }
    private:
        CJsonOptions::_info_type m_info_type = CJsonOptions::_info_type::info_type_unknown;
        std::chrono::microseconds m_scan_period{};
    };
} // namespace cchips
