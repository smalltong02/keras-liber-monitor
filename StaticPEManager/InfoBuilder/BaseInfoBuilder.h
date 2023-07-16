#pragma once
#include "InfoBuilder.h"
#include "utils.h"

namespace fs = std::filesystem;

namespace cchips {
    class CBaseInfoBuilder : public CInfoBuilder
    {
    public:
        CBaseInfoBuilder() { UpdateInfoType(CJsonOptions::_info_type::info_type_base); }
        ~CBaseInfoBuilder() = default;

        bool Initialize(std::unique_ptr<CJsonOptions>& json_options) { 
            if (!json_options)
                return false;
            if (!json_options->GetOptionsInfo(GetInfoType(), std::pair<unsigned char*, size_t>(reinterpret_cast<unsigned char*>(&m_options_info), sizeof(m_options_info))))
                return false;
            return true; 
        }
        bool Scan(fs::path& file_path, CFileInfo& file_info) {
            try {
                std::unique_ptr<cchips::CRapidJsonWrapper> json_result;
                json_result = std::make_unique<cchips::CRapidJsonWrapper>("{}");
                if (!json_result)
                    return false;
                if (fs::is_regular_file(file_path)) {
                    if (contain_invalid_utf8char(file_path.string())) {
                        file_path = fs::path(stringto_hexstring(file_path.string()));
                    }
                    if (m_options_info.bfilename) {
                        json_result->AddTopMember("filename", cchips::RapidValue(file_path.filename().string().c_str(), json_result->GetAllocator()));
                    }
                    if (m_options_info.blocation) {
                        json_result->AddTopMember("filepath", cchips::RapidValue(file_path.parent_path().string().c_str(), json_result->GetAllocator()));
                    }
                    if (m_options_info.bfiletype) {
                        json_result->AddTopMember("filetype", cchips::RapidValue(file_info.GetFileType().c_str(), json_result->GetAllocator()));
                    }
                    if (m_options_info.bfilesize) {
                        json_result->AddTopMember("filesize", cchips::RapidValue(fs::file_size(file_path)));
                    }
                    if (m_options_info.bfileattribute) {
                        std::string attributes = GetAttributes(file_path.string());
                        json_result->AddTopMember("fileattributes", cchips::RapidValue(attributes.c_str(), json_result->GetAllocator()));
                    }
                }
                file_info.SetJsonBaseInfo(std::move(json_result));
                return true;
            }
            catch (const std::exception& e)
            {
            }
            return false;
        }

    private:
        std::string GetAttributes(const std::string& path) const {
            DWORD attrs = GetFileAttributes(path.c_str());
            std::string attrs_str;
            auto set_attrs_func = [](std::string& attrs_str, const std::string& attr) {
                if (attrs_str.length()) {
                    attrs_str += std::string(" | ") + attr;
                    return;
                }
                attrs_str = attr;
            };

            if (attrs & FILE_ATTRIBUTE_ARCHIVE) {
                set_attrs_func(attrs_str, "Archive");
            }
            if (attrs & FILE_ATTRIBUTE_COMPRESSED) {
                set_attrs_func(attrs_str, "Compressed");
            }
            if (attrs & FILE_ATTRIBUTE_ENCRYPTED) {
                set_attrs_func(attrs_str, "Encrypted");
            }
            if (attrs & FILE_ATTRIBUTE_HIDDEN) {
                set_attrs_func(attrs_str, "Hidden");
            }
            if (attrs & FILE_ATTRIBUTE_NORMAL) {
                set_attrs_func(attrs_str, "Normal");
            }
            if (attrs & FILE_ATTRIBUTE_READONLY) {
                set_attrs_func(attrs_str, "Readonly");
            }
            if (attrs & FILE_ATTRIBUTE_SYSTEM) {
                set_attrs_func(attrs_str, "System");
            }
            if (attrs & FILE_ATTRIBUTE_TEMPORARY) {
                set_attrs_func(attrs_str, "Temporary");
            }

            if (!attrs_str.length())
                attrs_str = "Unknown";
            return attrs_str;
        }
        CJsonOptions::_base_info m_options_info;
    };
} // namespace cchips
