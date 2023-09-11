#pragma once
#include <string>
#include <filesystem>
#include <fstream>
#include <memory>
#include <functional>
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

        void Initialize(const std::string& input) {
            m_input_path = input;
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
                        else {
                            return;
                        }
                    }
                    else {
                        return;
                    }
                }
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
            if (m_type == input_memformat) {
                filebuffer = scanfile;
            }
            else {
                if (!LoadFile(scanfile, filebuffer) || !filebuffer.length()) {
                    return false;
                }
            }
            std::unique_ptr<CRapidJsonWrapper> wrapper = std::make_unique<CRapidJsonWrapper>(filebuffer);
            if (!wrapper || !wrapper->IsValid()) {
                return false;
            }
            std::stringstream datasets;
            switch (m_model) {
            case model_fasttext:
            {
                GeneratingFastTextDatasets(std::move(wrapper), datasets, m_label);
            }
            break;
            case model_gru:
            case model_lstm:
            {
                GeneratingGruDatasets(std::move(wrapper), datasets, m_label);
            }
            break;
            case model_transformers:
            {
                GeneratingTransformersDatasets(std::move(wrapper), datasets, m_label);
            }
            break;
            default:
                break;
            }
            if (m_type == input_jsonformat) {
                if (AppendLinetoFile(outputfile, datasets)) {
                    m_scan_counts++;
                    return true;
                }
            }
            else if (m_type == input_memformat) {
                outputfile = datasets.str();
                m_scan_counts++;
                return true;
            }

            return false;
        }

        bool AppendLinetoFile(const std::string& outputfile, const std::stringstream& datasets) const;
        bool GeneratingFastTextDatasets(std::unique_ptr<CRapidJsonWrapper>& wrapper, std::stringstream& datasets, const std::string& label) const;
        bool GeneratingGruDatasets(std::unique_ptr<CRapidJsonWrapper>& wrapper, std::stringstream& datasets, const std::string& label) const;
        bool GeneratingTransformersDatasets(std::unique_ptr<CRapidJsonWrapper>& wrapper, std::stringstream& datasets, const std::string& label) const;
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
        _inputtype m_type = input_jsonformat;
        _modeltype m_model = model_unknown;
        std::string m_input_path;
        std::string m_label;
        std::uint32_t m_scan_counts = 0;
        static const mongocxx::instance _mongodb_instance;
        static const std::vector <std::string> _special_vocab;
    };

#define GetCTextCorpusManager() CTextCorpusManager::GetInstance()
} // namespace cchips
