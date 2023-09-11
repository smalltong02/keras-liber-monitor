#pragma once
#include "InfoBuilder.h"
#include "PeInside/PeInside.h"

namespace cchips {
    class CPeinsideInfoBuilder : public CInfoBuilder
    {
    public:
        CPeinsideInfoBuilder() { UpdateInfoType(CJsonOptions::_info_type::info_type_peinside); }
        ~CPeinsideInfoBuilder() = default;

        bool Initialize(std::unique_ptr<CJsonOptions>& json_options) {
            if (!json_options)
                return false;
            if (!json_options->GetOptionsInfo(GetInfoType(), std::pair<unsigned char*, size_t>(reinterpret_cast<unsigned char*>(&m_options_info), sizeof(m_options_info))))
                return false;
            if (!m_pe_inside.Initialize(m_options_info))
                return false;
            m_valid = true;
            return m_valid;
        }
        bool Scan(const std::string& file_buf, CFileInfo& file_info) {
            try {
                if (!IsValid()) {
                    return false;
                }
                if (!m_pe_inside.Scan(file_buf)) {
                    return false;
                }
                auto result = m_pe_inside.MoveResult();
                if (result) {
                    file_info.SetPeInsideInfo(std::move(result));
                }
                return true;
            }
            catch (const std::exception& e)
            {
            }
            return false;
        }
        bool Scan(fs::path& file_path, CFileInfo& file_info) { 
            try {
                if (!IsValid()) {
                    return false;
                }
                if (!m_pe_inside.Scan(file_path)) {
                    return false;
                }
                auto result = m_pe_inside.MoveResult();
                if (result) {
                    file_info.SetPeInsideInfo(std::move(result));
                }
                return true;
            }
            catch (const std::exception& e)
            {
            }
            return false;
        }
        bool ScanPeriod(fs::path& file_path, CFileInfo& file_info) {
            try {
                if (!IsValid()) {
                    return false;
                }
                if (!m_pe_inside.ScanPeriod(file_path)) {
                    return false;
                }

                return true;
            }
            catch (const std::exception& e)
            {
            }
            return false;
        }
        bool IsValid() const { return m_valid; }
    private:
        bool m_valid = false;
        CJsonOptions::_peinside_info m_options_info{};
        CPeInside m_pe_inside;
    };
} // namespace cchips
