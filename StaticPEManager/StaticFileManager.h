#pragma once
#include <mongocxx/instance.hpp>
#include <mongocxx/client.hpp>
#include <bsoncxx/json.hpp>
#include <mongocxx/gridfs/bucket.hpp>
#include <bsoncxx/builder/stream/document.hpp>
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
        typedef struct {
            std::uint32_t format_success_count = 0;
            std::uint32_t inside_success_count = 0;
            std::uint32_t insnflow_success_count = 0;
            void clear() {
                format_success_count = 0;
                inside_success_count = 0;
                insnflow_success_count = 0;
            }
            std::uint32_t GetSuccessCount() const {
                return GetFormatSuccessCount();
            }
            std::uint32_t GetFormatSuccessCount() const {
                return format_success_count;
            }
            std::uint32_t GetInsideSuccessCount() const {
                return inside_success_count;
            }
            std::uint32_t GetInsnflowSuccessCount() const {
                return insnflow_success_count;
            }
            void IncSuccessCount() {
                return IncFormatSuccessCount();
            }
            void IncFormatSuccessCount() {
                format_success_count++;
            }
            void IncInsideSuccessCount() {
                inside_success_count++;
            }
            void IncInsnflowSuccessCount() {
                insnflow_success_count++;
            }
        } success_scan_count;

        static CStaticFileManager& GetInstance()
        {
            static CStaticFileManager m_instance;
            return m_instance;
        }

        bool Initialize(std::unique_ptr<CJsonOptions> options_ptr) {
            if (!options_ptr)
                return false;
            m_options = std::move(options_ptr);
            m_success_scan_count.clear();
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

        bool Scan(const std::string& scan_path, std::string& output, bool boutput_console = false) {
            m_boutput_console = boutput_console;
            if (!m_options) return false;
            if (!fs::exists(scan_path)) {
                error_log("scan_path %s is not exist!", scan_path.c_str());
                return false;
            }
            auto outputtype = m_options->GetConfigInfo().GetOutputType();
            if (outputtype == CJsonOptions::_config::_outputtype::output_jsonformat ||
                outputtype == CJsonOptions::_config::_outputtype::output_logformat) {
                if (!fs::exists(output)) {
                    error_log("output_path %s is not exist!", output.c_str());
                    return false;
                }
                if (!fs::is_directory(output)) {
                    error_log("output_path %s is not a directory!", output.c_str());
                    return false;
                }
            }
            if (fs::is_directory(scan_path)) {
                if (outputtype == CJsonOptions::_config::_outputtype::output_memformat) {
                    return false;
                }
                CTraverseDriveEngine::fi_callback callback = std::bind(&CStaticFileManager::scan_callback, this, std::placeholders::_1, std::placeholders::_2);
                CTraverseDriveEngine::GetInstance().AddFiCb(callback);
                return CTraverseDriveEngine::GetInstance().TraverseDrive(scan_path, output);
            }
            else if (fs::is_regular_file(scan_path)) {
                return scan_callback(scan_path, output);
            }
            else {
                return false;
            }
        }

        std::string GetSuccessScanCount() const {
            if (!m_options)
                return false;
            std::stringstream ss{""};
            switch (m_options->GetConfigInfo().GetOutputType()) {
            case CJsonOptions::_config::_outputtype::output_mongodbformat:
            {
                ss << "format_success_count: " << m_success_scan_count.GetFormatSuccessCount() << std::endl \
                   << "inside_success_count: " << m_success_scan_count.GetInsideSuccessCount() << std::endl \
                   << "insnflow_success_count: " << m_success_scan_count.GetInsnflowSuccessCount() << std::endl;
            }
            break;
            case CJsonOptions::_config::_outputtype::output_jsonformat:
            case CJsonOptions::_config::_outputtype::output_logformat:
            case CJsonOptions::_config::_outputtype::output_pipeformat:
            case CJsonOptions::_config::_outputtype::output_ipformat:
            case CJsonOptions::_config::_outputtype::output_memformat:
            default:
                ss << "success_count: " << m_success_scan_count.GetSuccessCount() << std::endl;
            }
            return ss.str();
        }
        void ClrSuccessScanCount() { m_success_scan_count.clear(); }

    private:
#define _mongodb_default_uri "mongodb://localhost:27017"
#define _mongodb_database_name "HipsCollector"
#define _mongodb_collector_format_info "PeFormatInfo"
#define _mongodb_collector_inside_info "PeinsideInfo"
#define _mongodb_collector_insnflow_info "PeinsnflowInfo"
#define _mongodb_document_limit 16777216

        CStaticFileManager() = default;
        ~CStaticFileManager() = default;
        bool scan_callback(const std::string& scan_path, std::string& output) {
            if (m_boutput_console) {
                std::cout << "scan: " << scan_path << std::endl;
            }
            if (!m_options) return false;
            std::unique_ptr<CFileInfo> fileinfo = std::make_unique<CFileInfo>(scan_path);
            if (!fileinfo) {
                return false;
            }
            if (!fileinfo->IsValid())
                return false;
            try {
                auto& config = m_options->GetConfigInfo();
                if (!config.MatchFileType(fileinfo->GetFileFormat())) {
                    return false;
                }
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
            case CJsonOptions::_config::_outputtype::output_memformat:
            {
                if (GenerateMemFormat(std::move(fileinfo), output)) {
                    m_success_scan_count.IncSuccessCount();
                    return true;
                }
            }
            break;
            case CJsonOptions::_config::_outputtype::output_jsonformat:
            {
                auto output_file = output + "\\" + fileinfo->GetIdentifierInfo().GetIdentifier() + ".json";
                if (GenerateJsonFormat(std::move(fileinfo), output_file)) {
                    m_success_scan_count.IncSuccessCount();
                    return true;
                }
            }
            break;
            case CJsonOptions::_config::_outputtype::output_mongodbformat:
            {
                if (GenerateMongodbFormat(std::move(fileinfo), output)) {
                    return true;
                }
            }
            break;
            case CJsonOptions::_config::_outputtype::output_logformat:
            case CJsonOptions::_config::_outputtype::output_pipeformat:
            case CJsonOptions::_config::_outputtype::output_ipformat:
            default:
                break;
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
            if (fileinfo->GetBaseInfo()) {
                auto basedoc = fileinfo->GetBaseInfo()->MoveDocument();
                if (basedoc) {
                    std::unique_ptr <cchips::RapidValue> baseinfo = std::make_unique<cchips::RapidValue>();
                    if (!baseinfo) return false;
                    baseinfo->CopyFrom(*basedoc, json_result->GetAllocator());
                    json_result->AddTopMember("baseinfo", std::move(baseinfo));
                }
            }
            if (fileinfo->GetVersionInfo()) {
                auto versiondoc = fileinfo->GetVersionInfo()->MoveDocument();
                if (versiondoc) {
                    std::unique_ptr <cchips::RapidValue> versioninfo = std::make_unique<cchips::RapidValue>();
                    if (!versioninfo) return false;
                    versioninfo->CopyFrom(*versiondoc, json_result->GetAllocator());
                    json_result->AddTopMember("versioninfo", std::move(versioninfo));
                }
            }
            if (fileinfo->GetInstalledInfo()) {
                auto installdoc = fileinfo->GetInstalledInfo()->MoveDocument();
                if (installdoc) {
                    std::unique_ptr <cchips::RapidValue> installinfo = std::make_unique<cchips::RapidValue>();
                    if (!installinfo) return false;
                    installinfo->CopyFrom(*installdoc, json_result->GetAllocator());
                    json_result->AddTopMember("installinfo", std::move(installinfo));
                }
            }
            if (fileinfo->GetVerifyInfo()) {
                auto signdoc = fileinfo->GetVerifyInfo()->MoveDocument();
                if (signdoc) {
                    std::unique_ptr <cchips::RapidValue> signinfo = std::make_unique<cchips::RapidValue>();
                    if (!signinfo) return false;
                    signinfo->CopyFrom(*signdoc, json_result->GetAllocator());
                    json_result->AddTopMember("signinfo", std::move(signinfo));
                }
            }
            if (fileinfo->GetPeInsideInfo()) {
                auto peinsidedoc = fileinfo->GetPeInsideInfo()->MoveDocument();
                if (peinsidedoc) {
                    std::unique_ptr <cchips::RapidValue> peinsideinfo = std::make_unique<cchips::RapidValue>();
                    if (!peinsideinfo) return false;
                    peinsideinfo->CopyFrom(*peinsidedoc, json_result->GetAllocator());
                    json_result->AddTopMember("insideinfo", std::move(peinsideinfo));
                }
            }
            if (fileinfo->GetInsnFlowInfo()) {
                auto insnflowdoc = fileinfo->GetInsnFlowInfo()->MoveDocument();
                if (insnflowdoc) {
                    std::unique_ptr <cchips::RapidValue> insnflowinfo = std::make_unique<cchips::RapidValue>();
                    if (!insnflowinfo) return false;
                    insnflowinfo->CopyFrom(*insnflowdoc, json_result->GetAllocator());
                    json_result->AddTopMember("insnflowinfo", std::move(insnflowinfo));
                }
            }
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

        bool GenerateMemFormat(std::unique_ptr<CFileInfo> fileinfo, std::string& output) {
            if (!fileinfo) return false;
            std::unique_ptr<cchips::CRapidJsonWrapper> json_result;
            json_result = std::make_unique<cchips::CRapidJsonWrapper>("{}");
            if (!json_result)
                return false;
            json_result->AddTopMember("sha256", fileinfo->GetIdentifierInfo().GetIdentifier());
            if (fileinfo->GetBaseInfo()) {
                auto basedoc = fileinfo->GetBaseInfo()->MoveDocument();
                if (basedoc) {
                    std::unique_ptr <cchips::RapidValue> baseinfo = std::make_unique<cchips::RapidValue>();
                    if (!baseinfo) return false;
                    baseinfo->CopyFrom(*basedoc, json_result->GetAllocator());
                    json_result->AddTopMember("baseinfo", std::move(baseinfo));
                }
            }
            if (fileinfo->GetVersionInfo()) {
                auto versiondoc = fileinfo->GetVersionInfo()->MoveDocument();
                if (versiondoc) {
                    std::unique_ptr <cchips::RapidValue> versioninfo = std::make_unique<cchips::RapidValue>();
                    if (!versioninfo) return false;
                    versioninfo->CopyFrom(*versiondoc, json_result->GetAllocator());
                    json_result->AddTopMember("versioninfo", std::move(versioninfo));
                }
            }
            if (fileinfo->GetInstalledInfo()) {
                auto installdoc = fileinfo->GetInstalledInfo()->MoveDocument();
                if (installdoc) {
                    std::unique_ptr <cchips::RapidValue> installinfo = std::make_unique<cchips::RapidValue>();
                    if (!installinfo) return false;
                    installinfo->CopyFrom(*installdoc, json_result->GetAllocator());
                    json_result->AddTopMember("installinfo", std::move(installinfo));
                }
            }
            if (fileinfo->GetVerifyInfo()) {
                auto signdoc = fileinfo->GetVerifyInfo()->MoveDocument();
                if (signdoc) {
                    std::unique_ptr <cchips::RapidValue> signinfo = std::make_unique<cchips::RapidValue>();
                    if (!signinfo) return false;
                    signinfo->CopyFrom(*signdoc, json_result->GetAllocator());
                    json_result->AddTopMember("signinfo", std::move(signinfo));
                }
            }
            if (fileinfo->GetPeInsideInfo()) {
                auto peinsidedoc = fileinfo->GetPeInsideInfo()->MoveDocument();
                if (peinsidedoc) {
                    std::unique_ptr <cchips::RapidValue> peinsideinfo = std::make_unique<cchips::RapidValue>();
                    if (!peinsideinfo) return false;
                    peinsideinfo->CopyFrom(*peinsidedoc, json_result->GetAllocator());
                    json_result->AddTopMember("insideinfo", std::move(peinsideinfo));
                }
            }
            if (fileinfo->GetInsnFlowInfo()) {
                auto insnflowdoc = fileinfo->GetInsnFlowInfo()->MoveDocument();
                if (insnflowdoc) {
                    std::unique_ptr <cchips::RapidValue> insnflowinfo = std::make_unique<cchips::RapidValue>();
                    if (!insnflowinfo) return false;
                    insnflowinfo->CopyFrom(*insnflowdoc, json_result->GetAllocator());
                    json_result->AddTopMember("insnflowinfo", std::move(insnflowinfo));
                }
            }
            if (auto serial_str = json_result->Serialize(); serial_str) {
                output = (*serial_str);
                return true;
            }
            return false;
        }

        bool GenerateMongodbFormat(std::unique_ptr<CFileInfo> fileinfo, const std::string& output) {
            try {
                std::string mongodb_name = _mongodb_default_uri;
                if (output.length() && output._Starts_with("mongodb://")) {
                    mongodb_name = output;
                }
                mongocxx::client client{mongocxx::uri{mongodb_name.c_str()}};
                if (!client) return false;
                mongocxx::database db = client[_mongodb_database_name];
                if (!db) return false;
                //auto bucket = db.gridfs_bucket();
                //if (!bucket) return false;
                mongocxx::collection coll_format = db[_mongodb_collector_format_info];
                if (!coll_format) return false;
                mongocxx::collection coll_inside = db[_mongodb_collector_inside_info];
                if (!coll_inside) return false;
                mongocxx::collection coll_insnflow = db[_mongodb_collector_insnflow_info];
                if (!coll_insnflow) return false;
                return GenerateMongodbDocument(fileinfo, coll_format, coll_inside, coll_insnflow);
            }
            catch (const std::exception& e) {
                std::cout << "Exception occurred during document generation: " << e.what() << std::endl;
            }
            return false;
        }

        bool GenerateMongodbDocument(std::unique_ptr<CFileInfo>& fileinfo, mongocxx::collection& coll_format, mongocxx::collection& coll_inside, mongocxx::collection& coll_insnflow) {
            if (!fileinfo) return false;

            std::unique_ptr<cchips::CRapidJsonWrapper> json_result;
            json_result = std::make_unique<cchips::CRapidJsonWrapper>("{}");
            if (!json_result)
                return false;
            std::string iden = fileinfo->GetIdentifierInfo().GetIdentifier();
            json_result->AddTopMember("sha256", iden);
            if (fileinfo->GetBaseInfo()) {
                auto basedoc = fileinfo->GetBaseInfo()->MoveDocument();
                if (basedoc) {
                    std::unique_ptr <cchips::RapidValue> baseinfo = std::make_unique<cchips::RapidValue>();
                    if (!baseinfo) return false;
                    baseinfo->CopyFrom(*basedoc, json_result->GetAllocator());
                    json_result->AddTopMember("baseinfo", std::move(baseinfo));
                }
            }
            if (fileinfo->GetVersionInfo()) {
                auto versiondoc = fileinfo->GetVersionInfo()->MoveDocument();
                if (versiondoc) {
                    std::unique_ptr <cchips::RapidValue> versioninfo = std::make_unique<cchips::RapidValue>();
                    if (!versioninfo) return false;
                    versioninfo->CopyFrom(*versiondoc, json_result->GetAllocator());
                    json_result->AddTopMember("versioninfo", std::move(versioninfo));
                }
            }
            if (fileinfo->GetInstalledInfo()) {
                auto installdoc = fileinfo->GetInstalledInfo()->MoveDocument();
                if (installdoc) {
                    std::unique_ptr <cchips::RapidValue> installinfo = std::make_unique<cchips::RapidValue>();
                    if (!installinfo) return false;
                    installinfo->CopyFrom(*installdoc, json_result->GetAllocator());
                    json_result->AddTopMember("installinfo", std::move(installinfo));
                }
            }
            if (fileinfo->GetVerifyInfo()) {
                auto signdoc = fileinfo->GetVerifyInfo()->MoveDocument();
                if (signdoc) {
                    std::unique_ptr <cchips::RapidValue> signinfo = std::make_unique<cchips::RapidValue>();
                    if (!signinfo) return false;
                    signinfo->CopyFrom(*signdoc, json_result->GetAllocator());
                    json_result->AddTopMember("signinfo", std::move(signinfo));
                }
            }
            if (AddJsonDocToCollection(iden, std::move(json_result), coll_format)) {
                m_success_scan_count.IncFormatSuccessCount();
            }
            
            json_result = std::make_unique<cchips::CRapidJsonWrapper>("{}");
            if (!json_result)
                return false;
            json_result->AddTopMember("sha256", iden);
            if (fileinfo->GetPeInsideInfo()) {
                auto peinsidedoc = fileinfo->GetPeInsideInfo()->MoveDocument();
                if (peinsidedoc) {
                    std::unique_ptr <cchips::RapidValue> peinsideinfo = std::make_unique<cchips::RapidValue>();
                    if (!peinsideinfo) return false;
                    peinsideinfo->CopyFrom(*peinsidedoc, json_result->GetAllocator());
                    json_result->AddTopMember("insideinfo", std::move(peinsideinfo));
                }
            }
            if (AddJsonDocToCollection(iden, std::move(json_result), coll_inside)) {
                m_success_scan_count.IncInsideSuccessCount();
            }

            json_result = std::make_unique<cchips::CRapidJsonWrapper>("{}");
            if (!json_result)
                return false;
            json_result->AddTopMember("sha256", iden);
            if (fileinfo->GetInsnFlowInfo()) {
                auto insnflowdoc = fileinfo->GetInsnFlowInfo()->MoveDocument();
                if (insnflowdoc) {
                    std::unique_ptr <cchips::RapidValue> insnflowinfo = std::make_unique<cchips::RapidValue>();
                    if (!insnflowinfo) return false;
                    insnflowinfo->CopyFrom(*insnflowdoc, json_result->GetAllocator());
                    json_result->AddTopMember("insnflowinfo", std::move(insnflowinfo));
                }
            }
            if (AddJsonDocToCollection(iden, std::move(json_result), coll_insnflow)) {
                m_success_scan_count.IncInsnflowSuccessCount();
                return true;
            }
            return false;
        }
        bool AddJsonDocToCollection(const std::string& iden, std::unique_ptr<cchips::CRapidJsonWrapper> json_result, mongocxx::collection& coll) {
            if (!json_result)
                return false;
            bsoncxx::builder::stream::document doc{};
            try {
                if (auto serial_str = json_result->Serialize(); serial_str) {
                    if (serial_str->length() >= _mongodb_document_limit) {
                        return false;
                    }
                    auto docvalue = bsoncxx::from_json((*serial_str));
                    for (const auto& element : docvalue.view()) {
                        doc << element.key() << element.get_value();
                    }
                    bsoncxx::builder::stream::document filter;
                    filter << "sha256" << iden.c_str();
                    auto result = coll.find_one(filter.view());
                    if (result.has_value()) {
                        coll.replace_one(filter.view(), doc.view());
                    }
                    else {
                        coll.insert_one(doc.view());
                    }
                }
            }
            catch (const std::exception& e) {
                std::cout << "Exception occurred during document insertion: " << e.what() << std::endl;
            }
            return true;
        }

        static const mongocxx::instance _mongodb_instance;
        std::unique_ptr<CJsonOptions> m_options = nullptr;
        std::unique_ptr<mongocxx::client> m_mongo_client = nullptr;
        std::vector<std::unique_ptr<CInfoBuilder>> m_infohandler;
        success_scan_count m_success_scan_count;
        std::uint32_t m_boutput_console = false;
    };

#define GetCStaticFileManager() CStaticFileManager::GetInstance()
} // namespace cchips
