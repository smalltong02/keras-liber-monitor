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
#include "InfoBuilder\PycBuilder.h"
#include "InfoBuilder\JavacBuilder.h"
#include "CPP/Common/Common.h"
#include "CPP/Common/MyWindows.h"
#include "C/CpuArch.h"
#include "CPP/Common/MyInitGuid.h"
#include "CPP/Common/CommandLineParser.h"
#include "CPP/Common/IntToString.h"
#include "CPP/Common/MyException.h"
#include "CPP/Common/StdInStream.h"
#include "CPP/Common/StdOutStream.h"
#include "CPP/Common/StringConvert.h"
#include "CPP/Common/StringToInt.h"
#include "CPP/Common/UTFConvert.h"
#include "CPP/Windows/ErrorMsg.h"
#include "CPP/Windows/TimeUtils.h"
#include "CPP/7zip/UI/Common/ArchiveCommandLine.h"
#include "CPP/7zip/UI/Common/Bench.h"
#include "CPP/7zip/UI/Common/ExitCode.h"
#include "CPP/7zip/UI/Common/Extract.h"
#ifdef EXTERNAL_CODECS
#include "CPP/7zip/UI/Common/LoadCodecs.h"
#endif
#include "CPP/7zip/Common/RegisterCodec.h"
#include "CPP/7zip/UI/Console/ExtractCallbackConsole.h"

extern const CExternalCodecs* g_ExternalCodecs_Ptr;

namespace cchips {

    namespace fs = std::filesystem;

    class CFuncduration {
    public:
        CFuncduration() : start_time_(std::chrono::high_resolution_clock::now()) {}
        CFuncduration(const std::string& prefix) : start_time_(std::chrono::high_resolution_clock::now()) {
            prefix_ = prefix;
        }

        ~CFuncduration() {
            const auto end_time = std::chrono::high_resolution_clock::now();
            const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time_);
            std::cout << prefix_ << formattime(duration) << std::endl;
        }

    private:
        std::string formattime(std::chrono::milliseconds time) {
            std::string duration_str;
            auto hours = std::chrono::duration_cast<std::chrono::hours>(time);
            time -= hours;
            auto minutes = std::chrono::duration_cast<std::chrono::minutes>(time);
            time -= minutes;
            auto seconds = std::chrono::duration_cast<std::chrono::seconds>(time);
            time -= seconds;
            auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(time);
            if (hours.count()) {
                duration_str = std::to_string(hours.count()) + " hours, ";
            }
            if (minutes.count()) {
                duration_str += std::to_string(minutes.count()) + " minutes, ";
            }
            if (seconds.count()) {
                duration_str += std::to_string(seconds.count()) + " seconds, ";
            }
            if (milliseconds.count()) {
                duration_str += std::to_string(milliseconds.count()) + " ms";
            }
            return duration_str;
        }
        std::string prefix_ = "running time: ";
        std::chrono::time_point<std::chrono::high_resolution_clock> start_time_;
    };

    class CDataCleaning {
    public:
        CDataCleaning(const fs::path& path) {
            if (path.empty()) {
                return;
            }
            m_hlogfile.open(path, std::ios::out | std::ios::trunc | std::ios::binary);
            if (!m_hlogfile.is_open()) {
                return;
            }
            m_valid = true;
        }
        ~CDataCleaning() {
            if (m_valid) {
                WriteJsonLog();
                m_hlogfile.close();
            }
        }
        bool AddPackageRecord(const fs::path& path) {
            if (!IsValid()) {
                return false;
            }
            if (path.empty()) {
                return false;
            }
            m_packagelists.push_back(path.string());
            return GenerateJsonLog();
        }
        bool AddInternalpkgRecord(const fs::path& path) {
            if (!IsValid()) {
                return false;
            }
            if (path.empty()) {
                return false;
            }
            m_internalpkglists.push_back(path.string());
            return GenerateJsonLog();
        }
        bool AddNormalRecord(const fs::path& path) {
            if (!IsValid()) {
                return false;
            }
            if (path.empty()) {
                return false;
            }
            m_normallists.push_back(path.string());
            return GenerateJsonLog();
        }
        bool AddSuccessCollectRecord(const fs::path& path, const fs::path& subpath) {
            if (!IsValid()) {
                return false;
            }
            if (path.empty()) {
                return false;
            }
            std::string combined_path;
            combined_path = path.string();
            m_successlists.push_back(path.string());
            std::string name = subpath.filename().string();
            AddDupCollectRecord(name, path);
            return GenerateJsonLog();
        }
        bool AddFailedCollectRecord(const fs::path& path, const fs::path& subpath) {
            if (!IsValid()) {
                return false;
            }
            if (path.empty()) {
                return false;
            }
            std::string combined_path;
            combined_path = path.string();
            m_failedlists.push_back(path.string());
            std::string name = subpath.filename().string();
            return GenerateJsonLog();
        }
        bool IsValid() const {
            return m_valid;
        }
    private:
        bool AddDupCollectRecord(const std::string& name, const fs::path& path) {
            if (name.empty() || path.empty()) {
                return false;
            }
            auto it = m_duplists.find(name);
            if (it == m_duplists.end()) {
                m_duplists[name] = std::vector<std::string>({ path.string() });
            }
            else {
                it->second.push_back(path.string());
            }
            return true;
        }

        bool GenerateJsonLog() {
            if (!IsValid()) {
                return false;
            }
            std::uint32_t success_counts = m_successlists.size();
            if (success_counts > 0 && success_counts % 100 != 0) {
                return true;
            }
            if (WriteJsonLog()) {
                return true;
            }
            return false;
        }
        bool WriteJsonLog() {
            std::uint32_t package_counts = m_packagelists.size();
            std::uint32_t internalpkg_counts = m_internalpkglists.size();
            std::uint32_t normal_counts = m_normallists.size();
            std::uint32_t failed_counts = m_failedlists.size();
            std::uint32_t success_counts = m_successlists.size();
            std::uint32_t dup_counts = 0;

            std::unique_ptr<cchips::CRapidJsonWrapper> jsonfile = std::make_unique<cchips::CRapidJsonWrapper>("{}");
            if (!jsonfile) {
                return false;
            }
            auto& allocator = jsonfile->GetAllocator();

            std::unique_ptr<cchips::RapidValue> package_array = std::make_unique<cchips::RapidValue>();
            if (!package_array) {
                return false;
            }
            package_array->SetArray();
            for (auto& entry : m_packagelists) {
                package_array->PushBack(cchips::RapidValue(entry.c_str(), allocator), allocator);
            }

            std::unique_ptr<cchips::RapidValue> internalpkg_array = std::make_unique<cchips::RapidValue>();
            if (!internalpkg_array) {
                return false;
            }
            internalpkg_array->SetArray();
            for (auto& entry : m_internalpkglists) {
                internalpkg_array->PushBack(cchips::RapidValue(entry.c_str(), allocator), allocator);
            }

            std::unique_ptr<cchips::RapidValue> normal_array = std::make_unique<cchips::RapidValue>();
            if (!normal_array) {
                return false;
            }
            normal_array->SetArray();
            for (auto& entry : m_normallists) {
                normal_array->PushBack(cchips::RapidValue(entry.c_str(), allocator), allocator);
            }

            std::unique_ptr<cchips::RapidValue> success_array = std::make_unique<cchips::RapidValue>();
            if (!success_array) {
                return false;
            }
            success_array->SetArray();
            for (auto& entry : m_successlists) {
                success_array->PushBack(cchips::RapidValue(entry.c_str(), allocator), allocator);
            }

            std::unique_ptr<cchips::RapidValue> failed_array = std::make_unique<cchips::RapidValue>();
            if (!failed_array) {
                return false;
            }
            failed_array->SetArray();
            for (auto& entry : m_failedlists) {
                failed_array->PushBack(cchips::RapidValue(entry.c_str(), allocator), allocator);
            }

            std::unique_ptr<cchips::RapidValue> dup_lists = std::make_unique<cchips::RapidValue>();
            if (!dup_lists) {
                return false;
            }
            dup_lists->SetObject();
            for (auto& entry : m_duplists) {
                if (entry.second.size() >= 2) {
                    dup_counts++;
                    cchips::RapidValue dup_object;
                    dup_object.SetObject();
                    cchips::RapidValue dup_array;
                    dup_array.SetArray();
                    for (auto& it : entry.second) {
                        dup_array.PushBack(cchips::RapidValue(it.c_str(), allocator), allocator);
                    }
                    dup_object.AddMember("dup_counts", entry.second.size(), allocator);
                    dup_object.AddMember("dup_paths", dup_array, allocator);
                    dup_lists->AddMember(cchips::RapidValue(entry.first.c_str(), allocator), dup_object, allocator);
                }
            }

            jsonfile->AddTopMember("package_counts", RapidValue(package_counts));
            jsonfile->AddTopMember("internalpkg_counts", RapidValue(internalpkg_counts));
            jsonfile->AddTopMember("normal_counts", RapidValue(normal_counts));
            jsonfile->AddTopMember("failed_counts", RapidValue(failed_counts));
            jsonfile->AddTopMember("success_counts", RapidValue(success_counts));
            jsonfile->AddTopMember("dup_counts", RapidValue(dup_counts));
            jsonfile->AddTopMember("package_lists", std::move(package_array));
            jsonfile->AddTopMember("internalpkg_lists", std::move(internalpkg_array));
            jsonfile->AddTopMember("normal_lists", std::move(normal_array));
            jsonfile->AddTopMember("success_lists", std::move(success_array));
            jsonfile->AddTopMember("failed_lists", std::move(failed_array));
            jsonfile->AddTopMember("dup_lists", std::move(dup_lists));
            if (auto serial_str = jsonfile->Serialize(); serial_str) {
                m_hlogfile.seekp(0, std::ios::beg);
                m_hlogfile << (*serial_str);
                return true;
            }
            return false;
        }
        bool m_valid = false;
        std::vector<std::string> m_packagelists;
        std::vector<std::string> m_internalpkglists;
        std::vector<std::string> m_normallists;
        std::vector<std::string> m_failedlists;
        std::vector<std::string> m_successlists;
        std::map<std::string, std::vector<std::string>> m_duplists;
        std::ofstream m_hlogfile;
    };

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
            m_temp_output = output;
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
                if (OutputConsoleFlag()) {
                    std::cout << "scan folder: " << scan_path << std::endl;
                }
                CTraverseDriveEngine::fi_callback callback = std::bind(&CStaticFileManager::scan_callback, this, std::placeholders::_1, std::placeholders::_2);
                CTraverseDriveEngine::GetInstance().AddFiCb(callback);
                CTraverseDriveEngine::GetInstance().EnableHierarchicalFolder();
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

        std::string GetSuccessScanCount() const {
            if (!m_options)
                return {};
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
        bool EnablePackage() {
            if (InitCodecs()) {
                m_bpackage = true;
            }
            return m_bpackage;
        }
        void EnableCleaning() {
            auto path = fs::current_path().append("datacleaning_log.json");
            m_data_cleaning = std::make_unique<CDataCleaning>(path);
            if (m_data_cleaning) {
                m_bcleaning = true;
            }
        }
        bool DisablePackage() {
            if (DeinitCodes()) {
                m_bpackage = false;
            }
            return !m_bpackage;
        }
        void DisableCleaning() {
            m_bcleaning = false;
            m_data_cleaning = nullptr;
        }
        void SetDefaultPwd(const std::wstring& pwd) {
            if (!pwd.empty()) {
                m_defaultpwd = FString(pwd.c_str());
            }
        }

    private:
#define _mongodb_default_uri "mongodb://localhost:27017"
#define _mongodb_database_name "HipsCollector"
#define _mongodb_collector_format_info "PeFormatInfo"
#define _mongodb_collector_inside_info "PeinsideInfo"
#define _mongodb_collector_insnflow_info "PeinsnflowInfo"
#define _mongodb_document_limit 16777216
#define _max_file_size 512 * 1024 * 1024 // 512M

        CStaticFileManager() = default;
        ~CStaticFileManager() = default;
        bool PackageFlag() { return m_bpackage; }
        bool CleaningFlag() { return (m_bcleaning && m_data_cleaning != nullptr); }
        bool OutputConsoleFlag() { return m_boutput_console; }
        static bool c7z_callback(const FString& path, CReadArcItem& item, const std::vector<unsigned char>& data);
        static bool extractc7z_format(std::uint32_t formatindex);
        static bool is_suffix_compress(const fs::path& extension);
        bool InitCodecs() {
            m_codecs = new CCodecs;
            if (!m_codecs) return false;
            m_codecsReleaser.Set(m_codecs);
            if (m_codecs->Load() != S_OK) return false;
            m_externalCodecs.GetCodecs = m_codecs;
            m_externalCodecs.GetHashers = m_codecs;
            Codecs_AddHashArcHandler(m_codecs);
            FString ferr;
            m_codecs->GetCodecsErrorMessage(ferr);
            if (!ferr.IsEmpty())
                return false;
            g_ExternalCodecs_Ptr = &m_externalCodecs;
            return true;
        }
        bool DeinitCodes() {
            return false;
        }
        void IncPackagelog(const fs::path& path) {
            if (!CleaningFlag()) {
                return;
            }
            m_data_cleaning->AddPackageRecord(path);
        }
        void IncInternalpkglog(const fs::path& path) {
            if (!CleaningFlag()) {
                return;
            }
            m_data_cleaning->AddInternalpkgRecord(path);
        }
        void IncNormallog(const fs::path& path) {
            if (!CleaningFlag()) {
                return;
            }
            m_data_cleaning->AddNormalRecord(path);
        }
        void IncSuccesslog(const fs::path& path, const fs::path& subpath) {
            if (!CleaningFlag()) {
                return;
            }
            m_data_cleaning->AddSuccessCollectRecord(path, subpath);
        }
        void IncFailedlog(const fs::path& path, const fs::path& subpath) {
            if (!CleaningFlag()) {
                return;
            }
            m_data_cleaning->AddFailedCollectRecord(path, subpath);
        }
        FString string_to_fstring(const std::string& input)
        {
            if (input.empty()) {
                return {};
            }
            std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
            std::wstring tmp = converter.from_bytes(input);
            if (tmp.empty()) {
                return {};
            }
            return FString(tmp.c_str());
        }
        std::string fstring_to_string(const FString& input)
        {
            if (input.IsEmpty()) {
                return {};
            }
            std::wstring tmp = input.Ptr();
            std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
            std::string tmpstr = converter.to_bytes(tmp);
            if (tmpstr.empty()) {
                return {};
            }
            return tmpstr;
        }
        std::string GetCorrectPath(const std::string& path1, const fs::path& path2, const fs::path& subpath) const {
            if (path2.empty()) {
                return path1;
            }
            std::string path;
            if (subpath.empty()) {
                path = path2.string();
            }
            else {
                path = path2.string() + " --> extract: " + subpath.string();
            }
            return path;
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
            if (filesize >= _max_file_size)
                return false;
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
        bool scanpackage_data(const std::string& data, std::string& output, const fs::path& package_path = "", const fs::path& subpath = "") {
            auto random_file_path = [&]() ->fs::path {
                const char characters[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
                const int characters_len = sizeof(characters) - 1;
                std::random_device rd;
                std::mt19937 gen(rd());

                std::string file_name;
                for (int i = 0; i < 10; ++i) {
                    file_name += characters[gen() % characters_len];
                }
                //auto tmppath = fs::temp_directory_path().append(file_name);
                auto tmppath = fs::current_path().append(file_name);
                return tmppath;
            };
            if (package_path.empty()) {
                return false;
            }
            if (data.empty()) {
                IncFailedlog(package_path, subpath);
                return false;
            }
            auto tmppath = random_file_path();
            std::ofstream ifile(tmppath, std::ios::out | std::ios::binary);
            if (!ifile.is_open()) {
                IncFailedlog(package_path, subpath);
                return false;
            }
            ifile.write(data.data(), data.size());
            ifile.seekp(0, std::ios::beg);
            ifile.close();
            bool bret = scanpackage_callback(tmppath.string(), output, package_path, subpath);
            fs::remove(tmppath);
            return bret;
        }

        bool scanpackage_callback(const std::string& scan_path, std::string& output, const fs::path& package_path = "", const fs::path& subpath = "") {
            CObjectVector<COpenType> types;
            CIntVector excluded_formats;
            COpenOptions op;
            op.props = nullptr;
            op.codecs = m_codecs;
            op.types = &types;
            op.excludedFormats = &excluded_formats;
            op.stdInMode = false;
            op.stream = NULL;
            op.filePath = string_to_fstring(scan_path);
            CArchiveLink arcLink;
            if (arcLink.Open_Strict(op, nullptr) != S_OK) {
                if (package_path.empty()) {
                    return scanfile_callback(scan_path, output);
                }
                else {
                    IncFailedlog(GetCorrectPath(scan_path, package_path, subpath), subpath);
                    return false;
                }
            }
            CArc& arc = arcLink.Arcs.Back();
            if (!extractc7z_format(arc.FormatIndex)) {
                if (package_path.empty()) {
                    return scanfile_callback(scan_path, output);
                }
                else {
                    IncFailedlog(GetCorrectPath(scan_path, package_path, subpath), subpath);
                    return false;
                }
            }
            IInArchive* archive = arc.Archive;
            if (!archive) {
                if (package_path.empty()) {
                    return scanfile_callback(scan_path, output);
                }
                else {
                    IncFailedlog(GetCorrectPath(scan_path, package_path, subpath), subpath);
                    return false;
                }
            }
            if (package_path.empty()) {
                IncPackagelog(GetCorrectPath(scan_path, package_path, subpath));
            }
            else {
                IncInternalpkglog(GetCorrectPath(scan_path, package_path, subpath));
            }
            UInt32 numItems;
            CRecordVector<UInt32> realIndices;
            RINOK(archive->GetNumberOfItems(&numItems));
            if (numItems == 0) {
                IncSuccesslog(GetCorrectPath(scan_path, package_path, subpath), subpath);
                return true;
            }
            for (UInt32 i = 0; i < numItems; i++)
            {
                realIndices.Add(i);
            }
            CArchiveExtractCallback* ecs = new CArchiveExtractCallback;
            CMyComPtr<IArchiveExtractCallback> ec(ecs);
            ecs->InitForMulti(false,
                NExtract::NPathMode::kFullPaths,
                NExtract::NOverwriteMode::kOverwrite,
                NExtract::NZoneIdMode::kNone,
                false // keepEmptyDirParts
            );
            NWindows::NFile::NFind::CFileInfo fi;
            if (!fi.Find_FollowLink(us2fs(string_to_fstring(scan_path)))) {
                IncFailedlog(GetCorrectPath(scan_path, package_path, subpath), subpath);
                return false;
            }
            if (fi.IsDir()) {
                IncFailedlog(GetCorrectPath(scan_path, package_path, subpath), subpath);
                return false;
            }
            ecs->InitBeforeNewArchive();
            ecs->Set7zCallback(CStaticFileManager::c7z_callback);
            ecs->SetDefaultPassword(m_defaultpwd);
            ecs->SetPackagePath(string_to_fstring(GetCorrectPath(scan_path, package_path, subpath)));
            UStringVector removePathParts;
            const CExtractNtOptions ntOptions;
            CExtractCallbackConsole* ecb = new CExtractCallbackConsole;
            CMyComPtr<IFolderArchiveExtractCallback> extractCallback = ecb;
            ecs->Init(
                ntOptions,
                NULL,
                &arc,
                ecb,
                false, false,
                L"",
                removePathParts, false,
                fi.Size + arcLink.VolumesSize);
            ecs->EnableMemmode();
            m_temp_output = output;
            auto result = archive->Extract(&realIndices.Front(), realIndices.Size(), false, ec);
            if (result != S_OK) {
                return false;
            }
            return true;
        }

        bool scanfile_callback(const std::string& scan_path, std::string& output) {
            std::string buffer;
            IncNormallog(scan_path);
//            if (LoadFile(scan_path, buffer)) {
                return scanmem_callback(scan_path, output, true);
//            }
            return false;
        }

        bool scanmem_callback(const std::string& scan_buffer, std::string& output, bool bfilemode = false, const fs::path& package_path = "", const fs::path& subpath = "") {
            std::unique_ptr<CFileInfo> fileinfo = nullptr;
            if (bfilemode) {
                fs::path path = scan_buffer;
                fileinfo = std::make_unique<CFileInfo>(path);
                fileinfo->SetBasename(path.filename().string());
            }
            else {
                fileinfo = std::make_unique<CFileInfo>(scan_buffer);
                fs::path path = subpath;
                fileinfo->SetBasename(path.filename().string());
            }
            if (!fileinfo) {
                IncFailedlog(GetCorrectPath(scan_buffer, package_path, subpath), subpath);
                return false;
            }
            if (!fileinfo->IsValid()) {
                IncFailedlog(GetCorrectPath(scan_buffer, package_path, subpath), subpath);
                return false;
            }
            try {
                auto& config = m_options->GetConfigInfo();
                const std::string& format = fileinfo->GetFileFormat();
                if (config.MatchExtraType(format)) {
                    if (config.IsPythonFormat(format)) {
                        if (!m_pychandler) {
                            m_pychandler = std::make_unique<CPycBuilder>();
                        }
                        if (!m_pychandler) {
                            IncFailedlog(GetCorrectPath(scan_buffer, package_path, subpath), subpath);
                            return false;
                        }
                        m_pychandler->Initialize(m_options);
                        if (m_options->GetConfigInfo().IsPerformanceMode()) {
                            if (bfilemode) {
                                m_pychandler->ScanPeriod(fs::path(scan_buffer), *fileinfo);
                            }
                            else {
                                m_pychandler->ScanPeriod(scan_buffer, *fileinfo);
                            }
                        }
                        else {
                            if (bfilemode) {
                                m_pychandler->Scan(fs::path(scan_buffer), *fileinfo);
                            }
                            else {
                                m_pychandler->Scan(scan_buffer, *fileinfo);
                            }
                        }
                    }
                    else if (config.IsJavaFormat(format)) {
                        if (!m_javachandler) {
                            m_javachandler = std::make_unique<CJavacBuilder>();
                        }
                        if (!m_javachandler) {
                            IncFailedlog(GetCorrectPath(scan_buffer, package_path, subpath), subpath);
                            return false;
                        }
                        m_javachandler->Initialize(m_options);
                        if (m_options->GetConfigInfo().IsPerformanceMode()) {
                            if (bfilemode) {
                                m_javachandler->ScanPeriod(fs::path(scan_buffer), *fileinfo);
                            }
                            else {
                                m_javachandler->ScanPeriod(scan_buffer, *fileinfo);
                            }
                        }
                        else {
                            if (bfilemode) {
                                m_javachandler->Scan(fs::path(scan_buffer), *fileinfo);
                            }
                            else {
                                m_javachandler->Scan(scan_buffer, *fileinfo);
                            }
                        }
                    }
                }
                else {
                    if (!config.MatchFileType(format)) {
                        IncFailedlog(GetCorrectPath(scan_buffer, package_path, subpath), subpath);
                        return false;
                    }
                    if (!config.MatchPeType(fileinfo->GetPeType())) {
                        IncFailedlog(GetCorrectPath(scan_buffer, package_path, subpath), subpath);
                        return false;
                    }
                    if (!config.MatchSubsystem(fileinfo->GetSubsystem())) {
                        IncFailedlog(GetCorrectPath(scan_buffer, package_path, subpath), subpath);
                        return false;
                    }

                    if (CleaningFlag()) {
                        IncSuccesslog(GetCorrectPath(scan_buffer, package_path, subpath), subpath);
                        m_success_scan_count.IncSuccessCount();
                        return true;
                    }
                    for (auto& handler : m_infohandler) {
                        if (!handler) continue;
                        if (m_options->GetConfigInfo().IsPerformanceMode()) {
                            if (bfilemode) {
                                handler->ScanPeriod(fs::path(scan_buffer), *fileinfo);
                            }
                            else {
                                handler->ScanPeriod(scan_buffer, *fileinfo);
                            }
                        }
                        else {
                            if (bfilemode) {
                                handler->Scan(fs::path(scan_buffer), *fileinfo);
                            }
                            else {
                                handler->Scan(scan_buffer, *fileinfo);
                            }
                        }
                    }
                }
            }
            catch (const std::exception& e) {
                IncFailedlog(GetCorrectPath(scan_buffer, package_path, subpath), subpath);
                return false;
            }
            switch (m_options->GetConfigInfo().GetOutputType()) {
            case CJsonOptions::_config::_outputtype::output_memformat:
            {
                if (GenerateMemFormat(std::move(fileinfo), output)) {
                    m_success_scan_count.IncSuccessCount();
                    IncSuccesslog(GetCorrectPath(scan_buffer, package_path, subpath), subpath);
                    return true;
                }
            }
            break;
            case CJsonOptions::_config::_outputtype::output_jsonformat:
            {
                auto output_file = output + "\\" + fileinfo->GetIdentifierInfo().GetIdentifier() + ".json";
                if (GenerateJsonFormat(std::move(fileinfo), output_file)) {
                    m_success_scan_count.IncSuccessCount();
                    IncSuccesslog(GetCorrectPath(scan_buffer, package_path, subpath), subpath);
                    return true;
                }
            }
            break;
            case CJsonOptions::_config::_outputtype::output_mongodbformat:
            {
                if (GenerateMongodbFormat(std::move(fileinfo), output)) {
                    IncSuccesslog(GetCorrectPath(scan_buffer, package_path, subpath), subpath);
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
            IncFailedlog(GetCorrectPath(scan_buffer, package_path, subpath), subpath);
            return false;
        }

        bool scan_callback(const std::string& scan_path, std::string& output) {
            if (!m_options) return false;
            if (OutputConsoleFlag()) {
                std::cout << "scan file: " << scan_path << std::endl;
            }
            if (PackageFlag()) {
                return scanpackage_callback(scan_path, output);
            }
            return scanfile_callback(scan_path, output);
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

        bool GenerateMongodbFormat(std::unique_ptr<CFileInfo> fileinfo, std::string& output) {
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

        bool m_bpackage = false;
        bool m_bcleaning = false;
        static const mongocxx::instance _mongodb_instance;
        static const std::map<std::uint32_t, bool> _codecs_formats;
        static const std::vector<std::string> _suffix_compress;
        std::unique_ptr<CJsonOptions> m_options = nullptr;
        std::unique_ptr<mongocxx::client> m_mongo_client = nullptr;
        std::vector<std::unique_ptr<CInfoBuilder>> m_infohandler;
        std::unique_ptr<CPycBuilder> m_pychandler = nullptr;
        std::unique_ptr<CJavacBuilder> m_javachandler = nullptr;
        success_scan_count m_success_scan_count;
        std::uint32_t m_boutput_console = false;
        std::string m_temp_output;
        FString m_defaultpwd = L"111";
        CCodecs* m_codecs = nullptr;
        CExternalCodecs m_externalCodecs;
        CCodecs::CReleaser m_codecsReleaser;
        std::unique_ptr<CDataCleaning> m_data_cleaning = nullptr;
    };

#define GetCStaticFileManager() CStaticFileManager::GetInstance()
} // namespace cchips
