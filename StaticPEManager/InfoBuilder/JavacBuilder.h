#pragma once
#include "InfoBuilder.h"

namespace cchips {

    class CJavacBuilder : public CInfoBuilder
    {
    public:
        CJavacBuilder() { UpdateInfoType(CJsonOptions::_info_type::info_type_javac); }
        ~CJavacBuilder() = default;
        bool Initialize(std::unique_ptr<CJsonOptions>& json_options);
        bool Scan(const std::string& file_buf, CFileInfo& file_info);
        bool Scan(fs::path& file_path, CFileInfo& file_info);
    private:
    };
} // namespace cchips
