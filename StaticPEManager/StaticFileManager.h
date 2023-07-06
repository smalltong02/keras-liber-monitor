#pragma once
#include "..\LogObject.h"
#include "TraverseDriveEngine.h"
#include "FileInfo.h"
#include "InfoBuilder\InfoBuilder.h"
#include "InfoBuilder\BaseInfoBuilder.h"
#include "InfoBuilder\VersionInfoBuilder.h"
#include "InfoBuilder\InstallInfoBuilder.h"
#include "InfoBuilder\SignatureInfoBuilder.h"
#include "InfoBuilder\PeinsideInfoBuilder.h"
#include "InfoBuilder\PeinsnflowInfoBuilder.h"

namespace cchips {

    namespace fs = std::filesystem;

    class CStaticFileManager {
    public:
        CStaticFileManager(std::unique_ptr<CJsonOptions> options_ptr) {
            Initialize(std::move(options_ptr));
        }
        ~CStaticFileManager() = default;

        bool Initialize(std::unique_ptr<CJsonOptions> options_ptr) {
            if (!options_ptr)
                return false;
            m_options = std::move(options_ptr);
            m_success_scan_count = 0;
            if (m_options->RequestBaseInfo()) {
                std::unique_ptr<CBaseInfoBuilder> info_builder_ptr = std::make_unique<CBaseInfoBuilder>();
                if (info_builder_ptr && info_builder_ptr->Initialize(m_options)) {
                    m_infohandler.push_back(std::move(info_builder_ptr));
                }
            }
            if (m_options->RequestVersionInfo()) {
                std::unique_ptr<CVersionInfoBuilder> ver_builder_ptr = std::make_unique<CVersionInfoBuilder>();
                if (ver_builder_ptr && ver_builder_ptr->Initialize(m_options)) {
                    m_infohandler.push_back(std::move(ver_builder_ptr));
                }
            }
            if (m_options->RequestInstallInfo()) {
                std::unique_ptr<CInstallInfoBuilder> install_builder_ptr = std::make_unique<CInstallInfoBuilder>();
                if (install_builder_ptr && install_builder_ptr->Initialize(m_options)) {
                    m_infohandler.push_back(std::move(install_builder_ptr));
                }
            }
            if (m_options->RequestSignatureInfo()) {
                std::unique_ptr<CSignatureInfoBuilder> signature_builder_ptr = std::make_unique<CSignatureInfoBuilder>();
                if (signature_builder_ptr && signature_builder_ptr->Initialize(m_options)) {
                    m_infohandler.push_back(std::move(signature_builder_ptr));
                }
            }
            if (m_options->RequestPeinsideInfo()) {
                std::unique_ptr<CPeinsideInfoBuilder> peinside_builder_ptr = std::make_unique<CPeinsideInfoBuilder>();
                if (peinside_builder_ptr && peinside_builder_ptr->Initialize(m_options)) {
                    m_infohandler.push_back(std::move(peinside_builder_ptr));
                }
            }
            if (m_options->RequestPeinsnflowInfo()) {
                std::unique_ptr<CPeinsnflowInfoBuilder> peinsnflow_builder_ptr = std::make_unique<CPeinsnflowInfoBuilder>();
                if (peinsnflow_builder_ptr && peinsnflow_builder_ptr->Initialize(m_options)) {
                    m_infohandler.push_back(std::move(peinsnflow_builder_ptr));
                }
            }
            return true;
        }

        bool Scan(const std::string& scan_path, const std::string& output_path) {
            if (!m_options) return false;
            if (!fs::exists(output_path)) {
                error_log("output_path %s is not exist!", output_path.c_str());
                return false;
            }
            if (!fs::is_directory(output_path)) {
                error_log("output_path %s is not a directory!", output_path.c_str());
                return false;
            }
            if (!fs::exists(scan_path)) {
                error_log("scan_path %s is not exist!", scan_path.c_str());
                return false;
            }
            if (fs::is_directory(scan_path)) {
                CTraverseDriveEngine::fi_callback callback = std::bind(&CStaticFileManager::scan_callback, this, std::placeholders::_1, std::placeholders::_2);
                CTraverseDriveEngine::GetInstance().AddFiCb(callback);
                return CTraverseDriveEngine::GetInstance().TraverseDrive(scan_path, output_path);
            }
            else if (fs::is_regular_file(scan_path)) {
                return scan_callback(scan_path, output_path);
            }
            else {
                return false;
            }
        }

        std::uint32_t GetSuccessScanCount() const { return m_success_scan_count; }

    private:
        bool scan_callback(const std::string& scan_path, const std::string& output_path) {
            if (!m_options) return false;
            std::unique_ptr<CFileInfo> fileinfo = std::make_unique<CFileInfo>(scan_path);
            if (!fileinfo) {
                return false;
            }
            if (!fileinfo->IsValid())
                return false;
            try {
                auto& config = m_options->GetConfigInfo();
                if (!config.MatchPeType(fileinfo->GetPeType())) {
                    return false;
                }
                if (!config.MatchSubsystem(fileinfo->GetSubsystem())) {
                    return false;
                }
            }
            catch (const std::exception& e) {
                return false;
            }
            for (auto& handler : m_infohandler) {
                if (!handler) continue;
                if (m_options->GetConfigInfo().IsPerformanceMode()) {
                    handler->ScanPeriod(fs::path(scan_path), *fileinfo);
                }
                else {
                    handler->Scan(fs::path(scan_path), *fileinfo);
                }
            }
            switch (m_options->GetConfigInfo().GetOutputType()) {
            case CJsonOptions::_config::_outputtype::output_jsonformat:
            {
                auto output_file = output_path + "\\" + fileinfo->GetIdentifierInfo().GetIdentifier() + ".json";
                if (GenerateJsonFormat(std::move(fileinfo), output_file)) {
                    m_success_scan_count++;
                    return true;
                }
            }
            break;
            case CJsonOptions::_config::_outputtype::output_logformat:
            case CJsonOptions::_config::_outputtype::output_pipeformat:
            case CJsonOptions::_config::_outputtype::output_ipformat:
            case CJsonOptions::_config::_outputtype::output_sqliteformat:
            default:
                ;
            }
            return false;
        }

        bool GenerateJsonFormat(std::unique_ptr<CFileInfo> fileinfo, const std::string& output_file) {
            if (!fileinfo) return false;
            std::unique_ptr<cchips::CRapidJsonWrapper> json_result;
            json_result = std::make_unique<cchips::CRapidJsonWrapper>("{}");
            if (!json_result)
                return false;
            json_result->AddTopMember("sha256", fileinfo->GetIdentifierInfo().GetIdentifier());
            std::unique_ptr <cchips::RapidValue> baseinfo = std::make_unique<cchips::RapidValue>();
            if (!baseinfo) return false;
            baseinfo->CopyFrom(*fileinfo->GetBaseInfo()->MoveDocument(), json_result->GetAllocator());
            json_result->AddTopMember("baseinfo", std::move(baseinfo));
            std::unique_ptr <cchips::RapidValue> versioninfo = std::make_unique<cchips::RapidValue>();
            if (!versioninfo) return false;
            versioninfo->CopyFrom(*fileinfo->GetVersionInfo()->MoveDocument(), json_result->GetAllocator());
            json_result->AddTopMember("versioninfo", std::move(versioninfo));
            std::unique_ptr <cchips::RapidValue> installinfo = std::make_unique<cchips::RapidValue>();
            if (!installinfo) return false;
            installinfo->CopyFrom(*fileinfo->GetInstalledInfo()->MoveDocument(), json_result->GetAllocator());
            json_result->AddTopMember("installinfo", std::move(installinfo));
            std::unique_ptr <cchips::RapidValue> signinfo = std::make_unique<cchips::RapidValue>();
            if (!signinfo) return false;
            signinfo->CopyFrom(*fileinfo->GetVerifyInfo()->MoveDocument(), json_result->GetAllocator());
            json_result->AddTopMember("signinfo", std::move(signinfo));
            std::unique_ptr <cchips::RapidValue> peinsideinfo = std::make_unique<cchips::RapidValue>();
            if (!peinsideinfo) return false;
            peinsideinfo->CopyFrom(*fileinfo->GetPeInsideInfo()->MoveDocument(), json_result->GetAllocator());
            json_result->AddTopMember("insideinfo", std::move(peinsideinfo));
            std::unique_ptr <cchips::RapidValue> insnflowinfo = std::make_unique<cchips::RapidValue>();
            if (!insnflowinfo) return false;
            //insnflowinfo->CopyFrom(*fileinfo->GetInsnFlowInfo()->MoveDocument(), json_result->GetAllocator());
            //json_result->AddTopMember("insnflowinfo", std::move(insnflowinfo));

            std::ofstream outputfile(output_file, std::ios::out | std::ios::trunc);
            if (!outputfile.is_open()) {
                return false;
            }
            if(auto serial_str = json_result->Serialize(); serial_str) {
                outputfile << (*serial_str);
                return true;
            }
            return false;
        }

        std::unique_ptr<CJsonOptions> m_options = nullptr;
        std::vector<std::unique_ptr<CInfoBuilder>> m_infohandler;
        std::uint32_t m_success_scan_count = 0;
    };
} // namespace cchips
