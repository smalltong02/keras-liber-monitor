#pragma once
#include <windows.h>
#include <iostream>
#include <filesystem>
#include <map>
#include <functional>
#include <fstream>
#include "utils.h"
#include "IdentifierInfo.h"
#include "osslsigncode.h"
#include "PackageWrapper.h"
#include "PeInfo.h"
#include "PeFormat.h"

using namespace osslsigncode;
namespace fs = std::filesystem;

namespace cchips {
    class CFileInfo
    {
    public:
        CFileInfo() = delete; 
        CFileInfo(const fs::path& path) {
            if (m_iden_info.Initialize(path, CIdentifierInfo::identifier_sha256)) {
                m_filetype_desc = GetFileType(path);
                m_fileformat_desc = FileDetector::GetFileFormat(path.string());
                std::unique_ptr<PeLib::PeFile> pe_file(FileDetector::ReadImageFile(path.string()));
                if (pe_file) {
                    tls_check_struct* tls = check_get_tls();
                    if (tls) {
                        tls->active = 1;
                        if (setjmp(tls->jb) == 0) {
                            pe_file->readMzHeader();
                            pe_file->readPeHeader();
                            tls->active = 0;
                        }
                        else {
                            //execption occur
                            tls->active = 0;
                            pe_file = nullptr;
                            return;
                        }
                    }
                    std::unique_ptr<PeFormat> pe_format = std::make_unique<PeFormat>(std::move(pe_file));
                    if (pe_format) {
                        m_subsystem_desc = pe_format->getSubsystemDesc();
                        m_petype_desc = "unknown";
                        if (pe_format->isExecutable()) {
                            m_petype_desc = "exe";
                        }
                        else if (pe_format->isDll()) {
                            m_petype_desc = "dll";
                        }
                        else if (pe_format->isSys()) {
                            m_petype_desc = "sys";
                        }
                    }
                }
                if (!m_filetype_desc.length() || !m_petype_desc.length())
                    return;
                m_valid = true;
            }
        }

        bool IsValid() const {
            return m_valid;
        }

        const std::string& GetFileType() { return m_filetype_desc; }
        const std::string& GetFileFormat() { return m_fileformat_desc; }
        const std::string& GetPeType() { return m_petype_desc; }
        const std::string& GetSubsystem() { return m_subsystem_desc; }
        CIdentifierInfo& GetIdentifierInfo() { return m_iden_info; }
        std::unique_ptr<cchips::CRapidJsonWrapper>& GetBaseInfo() { return m_json_baseinfo; }
        std::unique_ptr<cchips::CRapidJsonWrapper>& GetVersionInfo() { return m_json_versioninfo; }
        std::unique_ptr<cchips::CRapidJsonWrapper>& GetInstalledInfo() { return m_json_installinfo; }
        std::unique_ptr<cchips::CRapidJsonWrapper>& GetVerifyInfo() { return m_json_verifyinfo; }
        std::unique_ptr<cchips::CRapidJsonWrapper>& GetPeInsideInfo() { return m_json_peinside; }
        std::unique_ptr<cchips::CRapidJsonWrapper>& GetInsnFlowInfo() { return m_json_insnflow; }
        void SetPeInsideInfo(std::unique_ptr<cchips::CRapidJsonWrapper> peinside) { m_json_peinside = std::move(peinside); }
        void SetInsnFlowInfo(std::unique_ptr<cchips::CRapidJsonWrapper> insnflow) { m_json_insnflow = std::move(insnflow); }
        void SetJsonBaseInfo(std::unique_ptr<cchips::CRapidJsonWrapper> baseinfo) { m_json_baseinfo = std::move(baseinfo); }
        void SetJsonVersionInfo(std::unique_ptr<cchips::CRapidJsonWrapper> versioninfo) { m_json_versioninfo = std::move(versioninfo); }
        void SetJsonVerifyInfo(std::unique_ptr<cchips::CRapidJsonWrapper> verifyinfo) { m_json_verifyinfo = std::move(verifyinfo); }
        void SetJsonInstallInfo(std::unique_ptr<cchips::CRapidJsonWrapper> installinfo) { m_json_installinfo = std::move(installinfo); }
    private:
        std::string GetFileType(const fs::path& path);
        bool GetFileType(const std::string& sbuf, std::string& type, std::string& mime);

        bool m_valid = false;
        std::string m_filetype_desc;
        std::string m_petype_desc;
        std::string m_fileformat_desc;
        std::string m_subsystem_desc;
        CIdentifierInfo m_iden_info;
        std::unique_ptr<cchips::CRapidJsonWrapper> m_json_baseinfo;
        std::unique_ptr<cchips::CRapidJsonWrapper> m_json_versioninfo;
        std::unique_ptr<cchips::CRapidJsonWrapper> m_json_verifyinfo;
        std::unique_ptr<cchips::CRapidJsonWrapper> m_json_installinfo;
        std::unique_ptr<cchips::CRapidJsonWrapper> m_json_peinside;
        std::unique_ptr<cchips::CRapidJsonWrapper> m_json_insnflow;
    };
} // namespace cchips
