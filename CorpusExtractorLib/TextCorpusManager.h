#pragma once
#include <string>
#include <filesystem>
#include <fstream>
#include <memory>
#include <functional>
#include <set>
#include "extractor.h"
#include "Word2Vec.h"
#include <mongocxx/instance.hpp>
#include <mongocxx/client.hpp>
#include <bsoncxx/json.hpp>
#include <mongocxx/gridfs/bucket.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include "RapidJsonWrapper.h"
#include "StaticPeManager/TraverseDriveEngine.h"

namespace cchips {

    namespace fs = std::filesystem;

    class CSemanticGeneration {
    public:
        virtual ~CSemanticGeneration() = default;
        virtual bool addElement(const std::string& key, const std::string& value) = 0;
        virtual bool addElement(const std::string& key, bool value) = 0;
        virtual bool addElement(const std::string& key, std::uint32_t value) = 0;
        virtual bool addElement(const std::string& key, std::unique_ptr<CSemanticGeneration> value) = 0;
        virtual bool addElement(const std::string& key, const std::vector<std::pair<std::string, std::any>>& value_vec) = 0;
        virtual std::unique_ptr<CSemanticGeneration> makeSubSg() const = 0;
        virtual std::string fulldump() const = 0;
        virtual std::string justdump() const = 0;
        virtual std::uint32_t getelemcount() const = 0;
    private:
    };

    class CTextCorpusManager {
    private:
#define _mongodb_database_name "HipsCollector"
#define _mongodb_collector_format_info "PeFormatInfo"
#define _mongodb_collector_inside_info "PeinsideInfo"
#define _mongodb_collector_insnflow_info "PeinsnflowInfo"
#define _mongodb_corpusext_format "PeFormatCorpusext"
#define _mongodb_corpusext_inside "PeinsideCorpusext"
#define _mongodb_corpusext_insnflow "PeinsnflowCorpusext"
#define _mongodb_textcorpus_dict "HipsTextCorpusDict"
#define _max_file_size 512 * 1024 * 1024 // 512M
#define _maximum_feature_counts 5000
#define _minimum_feature_counts 1500
#define _minimum_word_counts 200

        CTextCorpusManager() = default;
        ~CTextCorpusManager() = default;

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

        bool IsJsonFile(const std::string& buffer) const {
            CRapidJsonWrapper wrapper(buffer);
            if (wrapper.IsValid()) {
                return true;
            }
            return false;
        }

    public:
        using _inputtype = enum {
            input_logformat = 0,
            input_jsonformat,
            input_mongodbformat,
            input_memformat,
        };
        using _modeltype = enum {
            model_unknown = 0,
            model_fasttext,
            model_gru,
            model_lstm,
            model_transformers,
        };

        static CTextCorpusManager& GetInstance()
        {
            static CTextCorpusManager m_instance;
            return m_instance;
        }

        void Initialize(const std::string& input, bool bdefflt = false) {
            m_input_path = input;
            m_labeldeep = 0;
            m_label.clear();
            if (!m_input_path.length()) {
                return;
            }
            if (m_input_path._Starts_with("mongodb://")) {
                m_type = input_mongodbformat;
            }
            else {
                if (!fs::exists(input)) {
                    if (IsJsonFile(input)) {
                        m_type = input_memformat;
                    }
                    else {
                        return;
                    }
                }
                else {
                    std::string path = input;
                    if (fs::is_directory(input)) {
                        for (auto& de : fs::directory_iterator(input)) {
                            if (!de.is_directory()) {
                                path = de.path().string();
                                break;
                            }
                        }
                    }
                    std::string buffer;
                    if (LoadFile(path, buffer)) {
                        if (IsJsonFile(buffer)) {
                            m_type = input_jsonformat;
                        }
                    }
                }
            }
            if (bdefflt) {
                InitFilterList();
            }
            if (!CStrExtractor::Initialize())
                return;
            m_valid = true;
            return;
        }

        bool WordEmbedding() const {
            if (m_type != input_mongodbformat) {
                
            }

            return true;
        }

        void labeldeepFlag(std::uint32_t labeldeep) {
            m_labeldeep = labeldeep;
        }

        bool GeneratingModelDatasets(const std::string& modelcorpus, std::string& output, const std::string& label) {
            bool bret = false;
            _modeltype model = GetModelCorpus(modelcorpus);
            if (model == model_unknown)
                return false;
            if (!output.length() && m_type != input_memformat)
                return false;
            m_model = model;
            m_label = label;
            if (m_type == input_mongodbformat) {

            }
            else if (m_type == input_jsonformat) {
                if (fs::is_directory(m_input_path)) {
                    CTraverseDriveEngine::fi_callback callback = std::bind(&CTextCorpusManager::scan_callback, this, std::placeholders::_1, std::placeholders::_2);
                    CTraverseDriveEngine::GetInstance().AddFiCb(callback);
                    bret = CTraverseDriveEngine::GetInstance().TraverseDrive(m_input_path, output);
                    std::cout << "Scan files: " << CTraverseDriveEngine::GetInstance().GetFileTotals() << std::endl;
                }
                else if (fs::is_regular_file(m_input_path)) {
                    bret = scan_callback(m_input_path, output);
                }
                else {
                    return false;
                }
            }
            else if (m_type == input_memformat) {
                bret = scan_callback(m_input_path, output);
            }
            //std::cout << "Generation datasets: " << m_scan_counts << std::endl;
            return bret;
        }

        bool CorpusExtract() const;
        bool AddExtraFltList(const fs::path& path) {
            if (path.empty()) {
                return true;
            }
            if (!fs::is_regular_file(path)) {
                return false;
            }
            auto filesize = fs::file_size(path);
            if (filesize == static_cast<uintmax_t>(-1) || filesize == 0) {
                return false;
            }
            std::ifstream infile;
            infile.open(path, std::ios::in | std::ios::binary);
            if (!infile.is_open()) {
                return false;
            }
            std::string line;
            try {
                while (std::getline(infile, line)) {
                    if (!line.empty() && line.back() == '\r') {
                        line.pop_back();
                    }
                    AddtoFilterList(line);
                }
            }
            catch (const std::exception& e) {
                infile.close();
                return false;
            }
            infile.close();
            return true;
        }
    private:
        class CCorpusExtractSG : public CSemanticGeneration {
        public:
            CCorpusExtractSG() {}
            ~CCorpusExtractSG() = default;
            bool initDict(const mongocxx::database& db);
            bool initDict(const mongocxx::collection& dict);
            bool addElement(const std::string& key, const std::string& value);
            bool addElement(const std::string& key, bool value);
            bool addElement(const std::string& key, std::uint32_t value);
            bool addElement(const std::string& key, std::unique_ptr<CSemanticGeneration> value);
            bool addElement(const std::string& key, const std::vector<std::pair<std::string, std::any>>& value_vec);
            std::unique_ptr<CSemanticGeneration> makeSubSg() const;
            std::string fulldump() const;
            std::string justdump() const;
            std::uint32_t getelemcount() const {
                return 0;
            }
        private:
            bool AddCorpus(const std::string& word, mongocxx::collection& dict, bool blower = true) const;
            void SpecialCorpusExtract(mongocxx::collection& dict) const;
            void ConstCorpusExtract(mongocxx::collection& dict) const;

            bool m_valid = false;
            mongocxx::collection m_textcorpus_dict;
        };

        class CFastTextSG : public CSemanticGeneration {
        public:
            CFastTextSG() { m_dataset << SPECIAL_CORPUS_CLS; }
            ~CFastTextSG() = default;
            bool addElement(const std::string& key, const std::string& value);
            bool addElement(const std::string& key, bool value);
            bool addElement(const std::string& key, std::uint32_t value);
            bool addElement(const std::string& key, std::unique_ptr<CSemanticGeneration> value);
            bool addElement(const std::string& key, const std::vector<std::pair<std::string, std::any>>& value_vec);
            std::unique_ptr<CSemanticGeneration> makeSubSg() const;
            std::string fulldump() const;
            std::string justdump() const;
            std::uint32_t getelemcount() const {
                return 0;
            }
        private:
            std::string getPrefix() const {
                std::string prefix;
                if (!m_append) {
                    prefix = " ";
                }
                else {
                    prefix = std::string(" ") + SPECIAL_CORPUS_COMMA + std::string(" ");
                }
                return prefix;
            }
            bool m_append = false;
            std::stringstream m_dataset;
        };

        std::string GetLabels(const fs::path& path) const {
            if (!m_label.empty()) {
                return m_label;
            }
            int labeldeep = m_labeldeep;
            if (!labeldeep) {
                return {};
            }
            if (path.empty()) {
                return {};
            }
            fs::path relpath = path.lexically_relative(m_input_path);
            std::string labels;
            std::string pre_part;
            labeldeep--;
            for (const auto& part : relpath) {
                if (part.empty()) {
                    continue;
                }
                if (part.compare("\\") == 0) {
                    continue;
                }
                if (!labeldeep) {
                    if (!pre_part.empty()) {
                        if (labels.empty()) {
                            labels = "__label_" + pre_part + "__";
                        }
                        else {
                            labels += " __label_" + pre_part + "__";
                        }
                    }
                    pre_part = part.string();
                    std::replace(pre_part.begin(), pre_part.end(), ' ', '-');
                }
                else {
                    labeldeep--;
                }
            }
            return labels;
        }

        void InitFilterList() {
            m_filter_defaultlist.insert("setup.exe");
            m_filter_defaultlist.insert("install.exe");
            m_filter_defaultlist.insert("uninstall.exe");
            m_filter_defaultlist.insert("update.exe");
            m_filter_defaultlist.insert("sqlite3.exe");
            m_filter_defaultlist.insert("php.exe");
            m_filter_defaultlist.insert("phpdbg.exe");
            m_filter_defaultlist.insert("php-win.exe");
            m_filter_defaultlist.insert("php-cgi.exe");
            m_filter_defaultlist.insert("perl.exe");
            m_filter_defaultlist.insert("openssl.exe");
            m_filter_defaultlist.insert("objdump.exe");
            m_filter_defaultlist.insert("objcopy.exe");
            m_filter_defaultlist.insert("mysqlimport.exe");
            m_filter_defaultlist.insert("mysqldump.exe");
            m_filter_defaultlist.insert("mysqld.exe");
            m_filter_defaultlist.insert("mysqlcheck.exe");
            m_filter_defaultlist.insert("mysqladmin.exe");
            m_filter_defaultlist.insert("mysql_plugin.exe");
            m_filter_defaultlist.insert("mysql.exe");
            m_filter_defaultlist.insert("javaw.exe");
            m_filter_defaultlist.insert("java.exe");
            m_filter_defaultlist.insert("httpd.exe");
            m_filter_defaultlist.insert("httxt2dbm.exe");
            m_filter_defaultlist.insert("htpasswd.exe");
            m_filter_defaultlist.insert("helper.exe");
            m_filter_defaultlist.insert("gzip.exe");
            m_filter_defaultlist.insert("ffmpeg.exe");
            m_filter_defaultlist.insert("bzip2.exe");
            m_filter_defaultlist.insert("apachemonitornt.exe");
            m_filter_defaultlist.insert("apachemonitor9x.exe");
            m_filter_defaultlist.insert("nuget.exe");
            m_filter_defaultlist.insert("7z.exe");
            m_filter_defaultlist.insert("zlib_solid");
            m_filter_defaultlist.insert("zlib");
            m_filter_defaultlist.insert("zip2exe.exe");
            m_filter_defaultlist.insert("zip.exe");
            m_filter_defaultlist.insert("yt-dlp.exe");
            m_filter_defaultlist.insert("youtube-dl.exe");
            m_filter_defaultlist.insert("xmlwf.exe");
            m_filter_defaultlist.insert("xmllint.exe");
            m_filter_defaultlist.insert("xmlcatalog.exe");
            m_filter_defaultlist.insert("xaralink.exe");
            m_filter_defaultlist.insert("wscript.exe");
            m_filter_defaultlist.insert("wperl.exe");
            m_filter_defaultlist.insert("wmiprvse.exe");
            m_filter_defaultlist.insert("wmic.exe");
            m_filter_defaultlist.insert("winlogon.exe");
            m_filter_defaultlist.insert("winmgmt.exe");
            m_filter_defaultlist.insert("wininst-9.0.exe");
            m_filter_defaultlist.insert("wininst-9.0-amd64.exe");
            m_filter_defaultlist.insert("wininst-8.0.exe");
            m_filter_defaultlist.insert("wininst-7.1.exe");
            m_filter_defaultlist.insert("wininst-6.exe");
            m_filter_defaultlist.insert("wininst-6.0.exe");
            m_filter_defaultlist.insert("wininst-14.0.exe");
            m_filter_defaultlist.insert("wininst-14.0-amd64.exe");
            m_filter_defaultlist.insert("wininst-10.0.exe");
            m_filter_defaultlist.insert("wininst-10.0-amd64.exe");
            m_filter_defaultlist.insert("winhlp32.exe");
            m_filter_defaultlist.insert("windres.exe");
            m_filter_defaultlist.insert("windmc.exe");
            m_filter_defaultlist.insert("wget.exe");
            m_filter_defaultlist.insert("wermgr.exe");
            m_filter_defaultlist.insert("werfault.exe");
            m_filter_defaultlist.insert("vcredist_x86_vs2013.exe");
            m_filter_defaultlist.insert("vcredist_x86_vs2010.exe");
            m_filter_defaultlist.insert("vcredist_x86.exe");
            m_filter_defaultlist.insert("vcredist_x64_vs2013.exe");
            m_filter_defaultlist.insert("vcredist_x64_vs2010.exe");
            m_filter_defaultlist.insert("vcredist_x64.exe");
            m_filter_defaultlist.insert("vcredist2010_x86.exe");
            m_filter_defaultlist.insert("vcredist100_x64.exe");
            m_filter_defaultlist.insert("vc_redist_2015.x86.exe");
            m_filter_defaultlist.insert("vc_redist_2015.x64.exe");
            m_filter_defaultlist.insert("vc_redist.x86.exe");
            m_filter_defaultlist.insert("vc_redist.x64.exe");
            m_filter_defaultlist.insert("vbrun60sp6.exe");
            m_filter_defaultlist.insert("vbrun60.exe");
            m_filter_defaultlist.insert("upx.exe");
            m_filter_defaultlist.insert("unzip.exe");
            m_filter_defaultlist.insert("unrar.exe");
            m_filter_defaultlist.insert("unpack200.exe");
            m_filter_defaultlist.insert("uninstaller.exe");
            m_filter_defaultlist.insert("uninstal.exe");
            m_filter_defaultlist.insert("uninst.exe");
            m_filter_defaultlist.insert("unins000.exe");
            m_filter_defaultlist.insert("tnameserv.exe");
            m_filter_defaultlist.insert("strings.exe");
            m_filter_defaultlist.insert("sqlite2.exe");
            m_filter_defaultlist.insert("sftp.exe");
            m_filter_defaultlist.insert("setup_x86.exe");
            m_filter_defaultlist.insert("setup_x64.exe");
            m_filter_defaultlist.insert("setup_wm.exe");
            m_filter_defaultlist.insert("setup64.exe");
            m_filter_defaultlist.insert("services.exe");
            m_filter_defaultlist.insert("scripts.exe");
            m_filter_defaultlist.insert("regsvr32.exe");
            m_filter_defaultlist.insert("regsvr.exe");
            m_filter_defaultlist.insert("rar.exe");
            m_filter_defaultlist.insert("qemu-system-x86_64.exe");
            m_filter_defaultlist.insert("qemu-system-mips64el.exe");
            m_filter_defaultlist.insert("qemu-system-aarch64.exe");
            m_filter_defaultlist.insert("pythonw.exe");
            m_filter_defaultlist.insert("python27.exe");
            m_filter_defaultlist.insert("python.exe");
            m_filter_defaultlist.insert("python-3.6.4-webinstall.exe");
            m_filter_defaultlist.insert("python-3.6.4-amd64-webinstall.exe");
            m_filter_defaultlist.insert("policytool.exe");
            m_filter_defaultlist.insert("pip.exe");
            m_filter_defaultlist.insert("perror.exe");
            m_filter_defaultlist.insert("perlglob.exe");
            m_filter_defaultlist.insert("pack200.exe");
            m_filter_defaultlist.insert("msiexec.exe");
            m_filter_defaultlist.insert("javaws.exe");
            m_filter_defaultlist.insert("javap.exe");
            m_filter_defaultlist.insert("javadoc.exe");
            m_filter_defaultlist.insert("javacpl.exe");
            m_filter_defaultlist.insert("javac.exe");
            m_filter_defaultlist.insert("java-rmi.exe");
            m_filter_defaultlist.insert("jarsigner.exe");
            m_filter_defaultlist.insert("jar.exe");
            m_filter_defaultlist.insert("installer.exe");
            m_filter_defaultlist.insert("hh.exe");
            m_filter_defaultlist.insert("grep.exe");
            m_filter_defaultlist.insert("gdb.exe");
            m_filter_defaultlist.insert("electron.exe");
            m_filter_defaultlist.insert("dpinst.exe");
            m_filter_defaultlist.insert("dpinst32.exe");
            m_filter_defaultlist.insert("dotnetfx.exe");
            m_filter_defaultlist.insert("dlltool.exe");
            m_filter_defaultlist.insert("devcon.exe");
            m_filter_defaultlist.insert("devchange.exe");
            m_filter_defaultlist.insert("curl.exe");
            m_filter_defaultlist.insert("cscript.exe");
        }
        bool AddtoFilterList(std::string& str) {
            if (str.empty()) {
                return false;
            }
            std::transform(str.begin(), str.end(), str.begin(),
                [](unsigned char c) { return std::tolower(c); }
            );
            m_filter_defaultlist.insert(str);
            return true;
        }
        bool InFilterList(const std::string& str) const {
            if (str.empty()) {
                return false;
            }
            auto it = m_filter_defaultlist.find(str);
            if (it == m_filter_defaultlist.end()) {
                return false;
            }
            return true;
        }

        class CTransformersSG : public CSemanticGeneration {
        public:
            CTransformersSG() = default;
            ~CTransformersSG() = default;
            bool addElement(const std::string& key, const std::string& value);
            bool addElement(const std::string& key, bool value);
            bool addElement(const std::string& key, std::uint32_t value);
            bool addElement(const std::string& key, std::unique_ptr<CSemanticGeneration> value);
            bool addElement(const std::string& key, const std::vector<std::pair<std::string, std::any>>& value_vec);
            std::unique_ptr<CSemanticGeneration> makeSubSg() const;
            std::string fulldump() const;
            std::string justdump() const;
            std::uint32_t getelemcount() const {
                return m_elementcount;
            }
        private:
            std::stringstream m_dataset;
            std::uint32_t m_elementcount = 0;
        };

        _modeltype GetModelCorpus(const std::string& model) const {
            if (!model.length()) {
                return model_unknown;
            }
            if (model.compare("fasttext") == 0) {
                return model_fasttext;
            }
            if (model.compare("gru") == 0) {
                return model_gru;
            }
            if (model.compare("lstm") == 0) {
                return model_lstm;
            }
            if (model.compare("transformers") == 0) {
                return model_transformers;
            }
            return model_unknown;
        }

        bool scan_callback(const std::string& scanfile, std::string& outputfile) {
            std::string filebuffer;
            std::string label;
            if (m_type == input_memformat) {
                filebuffer = scanfile;
                label = m_label;
            }
            else {
                if (!LoadFile(scanfile, filebuffer) || !filebuffer.length()) {
                    return false;
                }
                label = GetLabels(fs::path(scanfile).parent_path());
            }
            std::unique_ptr<CRapidJsonWrapper> wrapper = std::make_unique<CRapidJsonWrapper>(filebuffer);
            if (!wrapper || !wrapper->IsValid()) {
                return false;
            }
            bool bret = false;
            std::stringstream datasets;
            switch (m_model) {
            case model_fasttext:
            {
                bret = GeneratingFastTextDatasets(std::move(wrapper), datasets, label);
            }
            break;
            case model_gru:
            case model_lstm:
            {
                bret = GeneratingGruDatasets(std::move(wrapper), datasets, label);
            }
            break;
            case model_transformers:
            {
                bret = GeneratingTransformersDatasets(std::move(wrapper), datasets, label);
            }
            break;
            default:
                break;
            }
            if (bret) {
                if (m_type == input_jsonformat) {
                    if (AppendLinetoFile(outputfile, datasets)) {
                        SaveStatisticianData(outputfile);
                        return true;
                    }
                }
                else if (m_type == input_memformat) {
                    outputfile = datasets.str();
                    return true;
                }
            }
            return false;
        }
        bool SaveStatisticianData(const fs::path& outputfile) const;
        bool AppendLinetoFile(const std::string& outputfile, const std::stringstream& datasets) const;
        bool GeneratingFastTextDatasets(std::unique_ptr<CRapidJsonWrapper> wrapper, std::stringstream& datasets, const std::string& label);
        bool GeneratingGruDatasets(std::unique_ptr<CRapidJsonWrapper> wrapper, std::stringstream& datasets, const std::string& label);
        bool GeneratingTransformersDatasets(std::unique_ptr<CRapidJsonWrapper> wrapper, std::stringstream& datasets, const std::string& label);
        bool CorpusExtractPeFormat(mongocxx::collection& source, std::shared_ptr<CSemanticGeneration> sgobject) const;
        bool CorpusExtractPeInside(mongocxx::collection& source, std::shared_ptr<CSemanticGeneration> sgobject) const;
        bool CorpusExtractPeinsnflow(mongocxx::collection& source, std::shared_ptr<CSemanticGeneration> sgobject) const;
        bool IsPackData(double entropy) const { return (entropy >= 6.75); }
        bool MaybePackData(double entropy) const { return (entropy > 6.5 && entropy < 6.75); }
        bool UnpackData(double entropy) const { return (entropy <= 6.5); }
        bool CommonParseBaseInfo(ConstRapidObject& baseinfo, std::shared_ptr<CSemanticGeneration> sgobject) const;
        bool CommonParseVersionInfo(ConstRapidObject& versioninfo, std::shared_ptr<CSemanticGeneration> sgobject) const;
        bool CommonParseInstallInfo(ConstRapidObject& installinfo, std::shared_ptr<CSemanticGeneration> sgobject) const;
        bool CommonParseSigntureInfo(ConstRapidObject& signinfo, std::shared_ptr<CSemanticGeneration> sgobject) const;
        bool CommonParsePeInfo(ConstRapidObject& peinfo, std::shared_ptr<CSemanticGeneration> sgobject) const;
        bool CommonParseToolInfo(ConstRapidObject& toolinfo, std::shared_ptr<CSemanticGeneration> sgobject) const;
        bool CommonParseDataDirectoryInfo(ConstRapidObject& ddinfo, std::shared_ptr<CSemanticGeneration> sgobject) const;
        bool CommonParsePdbInfosInfo(ConstRapidObject& pdbinfo, std::shared_ptr<CSemanticGeneration> sgobject) const;
        bool CommonParseEmbeddedpeInfo(ConstRapidObject& embeddedinfo, std::shared_ptr<CSemanticGeneration> sgobject) const;
        bool CommonParseImportsInfo(ConstRapidObject& importsinfo, std::shared_ptr<CSemanticGeneration> sgobject) const;
        bool CommonParseExportsInfo(ConstRapidObject& exportsinfo, std::shared_ptr<CSemanticGeneration> sgobject) const;
        bool CommonParseManifestInfo(ConstRapidObject& manifestinfo, std::shared_ptr<CSemanticGeneration> sgobject) const;
        bool CommonParseResourceInfo(ConstRapidObject& manifestinfo, std::shared_ptr<CSemanticGeneration> sgobject) const;
        bool CommonParseRichheaderInfo(ConstRapidObject& richheaderinfo, std::shared_ptr<CSemanticGeneration> sgobject) const;
        bool CommonParseSectionsInfo(ConstRapidObject& sectionsinfo, std::shared_ptr<CSemanticGeneration> sgobject) const;
        bool CommonParseStrfeatureInfo(ConstRapidObject& strfeatureinfo, std::shared_ptr<CSemanticGeneration> sgobject, std::uint32_t feature_nums = _minimum_feature_counts) const;
        bool CommonParseFlowGraph(ConstRapidObject& flowgraphobj, std::shared_ptr<CSemanticGeneration> sgobject, std::uint32_t feature_nums = _minimum_feature_counts) const;
        bool CommonParseIlcode(ConstRapidObject& ilcodeobj, std::shared_ptr<CSemanticGeneration> sgobject, std::uint32_t feature_nums = _minimum_feature_counts) const;
        
        static const std::uint32_t _min_corpus_lenth;
        static const std::uint32_t _max_corpus_lenth;

        bool m_valid = false;
        std::uint32_t m_labeldeep = 0;
        std::uint32_t m_dup_idencounts = 0;
        _inputtype m_type = input_jsonformat;
        _modeltype m_model = model_unknown;
        std::string m_input_path;
        std::string m_label;
        std::vector<std::string> m_skip_lists;
        std::vector<std::string> m_notenough_lists;
        std::vector<std::string> m_scan_lists;
        std::set<std::string> m_scaned_idenlist;
        std::set<std::string> m_filter_defaultlist;
        static const mongocxx::instance _mongodb_instance;
        static const std::vector <std::string> _special_vocab;
    };

#define GetCTextCorpusManager() CTextCorpusManager::GetInstance()
} // namespace cchips
