#pragma once
#include "InfoBuilder.h"

namespace cchips {

    class CFileVersionInfo
    {
    public:
        CFileVersionInfo();
        virtual ~CFileVersionInfo();

    public:
        BOOL Create(HMODULE hModule = NULL);
        BOOL Create(LPCTSTR lpszFileName);

    public:
        WORD GetFileVersion(int nIndex) const;
        WORD GetProductVersion(int nIndex) const;
        DWORD GetFileFlagsMask() const;
        DWORD GetFileFlags() const;
        DWORD GetFileOs() const;
        DWORD GetFileType() const;
        DWORD GetFileSubtype() const;
        std::string GetFileTypeStr() const;

        std::string GetCompanyName() const;
        std::string GetFileDescription() const;
        std::string GetFileVersion() const;
        std::string GetInternalName() const;
        std::string GetLegalCopyright() const;
        std::string GetOriginalFileName() const;
        std::string GetProductName() const;
        std::string GetProductVersion() const;
        std::string GetComments() const;
        std::string GetLegalTrademarks() const;
        std::string GetPrivateBuild() const;
        std::string GetSpecialBuild() const;

        // implementation helpers
    protected:
        virtual void Reset();
        BOOL GetTranslationId(LPVOID lpData, UINT unBlockSize, WORD wLangId, DWORD& dwId, BOOL bPrimaryEnough = FALSE);

        BOOL RawGetFileInfo(LPVOID lpData, UINT unBlockSize);
        void SetFileInfo(PCWSTR pKey, PCWSTR pValue);

        // attributes
    private:
        VS_FIXEDFILEINFO m_FileInfo;

        std::string m_strCompanyName;
        std::string m_strFileDescription;
        std::string m_strFileVersion;
        std::string m_strInternalName;
        std::string m_strLegalCopyright;
        std::string m_strOriginalFileName;
        std::string m_strProductName;
        std::string m_strProductVersion;
        std::string m_strComments;
        std::string m_strLegalTrademarks;
        std::string m_strPrivateBuild;
        std::string m_strSpecialBuild;
    };

    class CVersionInfoBuilder : public CInfoBuilder
    {
    public:
        CVersionInfoBuilder() { UpdateInfoType(CJsonOptions::_info_type::info_type_version); }
        ~CVersionInfoBuilder() = default;

        bool Initialize(std::unique_ptr<CJsonOptions>& json_options) {
            if (!json_options)
                return false;
            if (!json_options->GetOptionsInfo(GetInfoType(), std::pair<unsigned char*, size_t>(reinterpret_cast<unsigned char*>(&m_options_info), sizeof(m_options_info))))
                return false;
            return true;
        }
        bool Scan(const std::string& file_buf, CFileInfo& file_info) {
            std::unique_ptr<cchips::CRapidJsonWrapper> json_result;
            json_result = std::make_unique<cchips::CRapidJsonWrapper>("{}");
            if (!json_result)
                return false;
            file_info.SetJsonVersionInfo(std::move(json_result));
            return true;
        }
        bool Scan(fs::path& file_path, CFileInfo& file_info) {
            try {
                std::unique_ptr<cchips::CRapidJsonWrapper> json_result;
                json_result = std::make_unique<cchips::CRapidJsonWrapper>("{}");
                if (!json_result)
                    return false;
                CFileVersionInfo ver_info;
                if (ver_info.Create(file_path.string().c_str())) {
                    if (ver_info.GetFileDescription().length() && m_options_info.bdescription) {
                        json_result->AddTopMember("description", cchips::RapidValue(ver_info.GetFileDescription().c_str(), json_result->GetAllocator()));
                    }
                    if (ver_info.GetFileTypeStr().length() && m_options_info.btype) {
                        json_result->AddTopMember("filetype", cchips::RapidValue(ver_info.GetFileTypeStr().c_str(), json_result->GetAllocator()));
                    }
                    if (ver_info.GetFileVersion().length() && m_options_info.bversion) {
                        json_result->AddTopMember("fileversion", cchips::RapidValue(ver_info.GetFileVersion().c_str(), json_result->GetAllocator()));
                    }
                    if (ver_info.GetProductName().length() && m_options_info.bproductname) {
                        json_result->AddTopMember("productname", cchips::RapidValue(ver_info.GetProductName().c_str(), json_result->GetAllocator()));
                    }
                    if (ver_info.GetProductVersion().length() && m_options_info.bproductversion) {
                        json_result->AddTopMember("productversion", cchips::RapidValue(ver_info.GetProductVersion().c_str(), json_result->GetAllocator()));
                    }
                    if (ver_info.GetLegalCopyright().length() && m_options_info.bcopyright) {
                        json_result->AddTopMember("copyright", cchips::RapidValue(ver_info.GetLegalCopyright().c_str(), json_result->GetAllocator()));
                    }
                }
                file_info.SetJsonVersionInfo(std::move(json_result));
                return true;
            }
            catch (std::filesystem::filesystem_error& e)
            {
            }
            return false;
        }

    private:
        CJsonOptions::_version_info m_options_info;
    };
} // namespace cchips
