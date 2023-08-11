#pragma once
#include <memory>
#include <filesystem>
#include <string>
#include <fstream>
#include <functional>
#include <map>
#include <opencv2/opencv.hpp>
#include <torch/torch.h>
#include <torch/data.h>
#include <torch/script.h>
#include <sentencepiece_processor.h>
#include "CorpusExtractorLib/Word2Vec.h"
#include "StaticPEManager/StaticFileManager.h"
#include "CorpusExtractorLib/TextCorpusManager.h"
#include "autotune.h"
#include "fasttext.h"

namespace cchips {

    namespace fs = std::filesystem;

    using ml_model = enum {
        mmodel_unknown,
        mmodel_fasttext,
        mmodel_gru,
        mmodel_lstm,
        mmodel_bert,
        mmodel_gpt,
    };

    class CGruModel;
    class CLstmModel;

    class CFunduration {
    public:
        CFunduration() : start_time_(std::chrono::high_resolution_clock::now()) {}
        CFunduration(const std::string& prefix) : start_time_(std::chrono::high_resolution_clock::now()) {
            prefix_ = prefix;
        }

        ~CFunduration() {
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

    class CDataSets : public torch::data::Dataset<CDataSets> {
    public:
#define _max_file_size 1024 * 1024 * 1024 // 1G
#define _max_batch_sequence_word 5000000
#define _max_sequences_size 10000
#define _max_word_dim 100

        using dict_type = enum {
            dict_word2vec,
            dict_fasttext,
            dict_sentencepiece,
        };

        CDataSets(const std::string& dataset_path, const std::string& dictbin_path, std::uint32_t ratio);
        ~CDataSets() = default;
        torch::data::Example<> get(size_t index) override;
        torch::optional<size_t> size() const override;
        std::unique_ptr<torch::Dict<std::string, torch::Tensor>> get_sp(size_t index) const;
        bool splitDatasets(const std::string& path);
        bool getTrainSets(std::vector<std::pair<std::string, std::string>>& train) const;
        bool getTestSets(std::vector<std::pair<std::string, std::string>>& test) const;
        void bagging();
        bool IsValid() const { return m_valid; }
        bool MakeDict(dict_type type = dict_word2vec);
        std::uint32_t getsequencesSize() const { return m_sequences_size; }
        std::uint32_t getWorddim() const { return m_word_dim; }
        std::uint32_t getLabelnum() const { return m_label.size(); }
        std::uint32_t getBatchsize() const { return m_batchsize; }
        std::string getLabel(std::uint32_t index) const {
            if (index >= m_label.size()) {
                return {};
            }
            for (auto& str : m_label) {
                if (index == 0) {
                    return str;
                }
                index--;
            }
            return {};
        }
        std::uint32_t findLabelidx(const std::string& label) const {
            std::uint32_t index = 0;
            if (!m_label.size()) return std::uint32_t(-1);
            for (auto& str : m_label) {
                if (str.compare(label) == 0) {
                    break;
                }
                index++;
            }
            if(index <= m_label.size())
                return index;
            return std::uint32_t(-1);
        }
        bool LoadDict(dict_type type = dict_word2vec);
        std::tuple<std::tuple<torch::Tensor, torch::Tensor>, torch::Tensor> LoadBatch(const std::vector<torch::data::Example<>>& batch) const;
        std::tuple<torch::Tensor, torch::Tensor> LoadSample(const std::string& input);
        bool save(std::ofstream& outfile);
        bool load(std::ifstream& infile);
        bool loadsentencemodel(std::ifstream& infile, std::int32_t filesize);
    private:
        void initLabelVec() {
            m_label.clear();
            m_label.insert("__label_cert_manage_tool__");
            m_label.insert("__label_forticlient__");
            m_label.insert("__label_putty__");
            m_label.insert("__label_teams__");
            m_label.insert("__label_unknown__");
            return;
        }

        bool LoadDataset(const std::string& path)
        {
            if (path.empty()) {
                return false;
            }
            if (!fs::exists(path)) {
                return false;
            }
            auto filesize = fs::file_size(path);
            if (filesize == static_cast<uintmax_t>(-1) || filesize == 0) {
                return false;
            }
            if (filesize >= _max_file_size)
                return false;
            std::ifstream infile;
            infile.open(path, std::ios::in | std::ios::binary);
            if (!infile.is_open()) {
                return false;
            }
            initLabelVec();
            m_dataset.clear();
            std::string line;
            std::uint32_t loading_count = 0;
            while (std::getline(infile, line)) {
                size_t pos = line.find_first_of(' ');
                if (pos != std::string::npos && pos != 0) {
                    //m_label.insert(line.substr(0, pos));
                    m_dataset.push_back(std::pair(line.substr(0, pos), line.substr(pos + 1)));
                }
                loading_count++;
                //if (loading_count >= 20) break;
            }
            std::cout << "LoadDataset: " << loading_count << std::endl;
            infile.close();
            m_path = path;
            return true;
        }
        bool MakeWord2vecDict();
        bool MakeFasttextDict();
        bool LoadWord2vecDict();
        bool LoadFasttextDict();
        bool LoadSentencepieceDict();

        bool m_valid = false;
        std::uint32_t m_word_dim = _max_word_dim;
        std::uint32_t m_sequences_size = _max_sequences_size;
        std::uint32_t m_batchsize = _max_batch_sequence_word / _max_sequences_size / _max_word_dim;
        dict_type m_dicttype = dict_type(-1);
        fs::path m_path;
        std::uint32_t m_ratio;
        size_t m_splitpos = 0;
        fs::path m_dictpath;
        std::set<std::string> m_label;
        std::map<std::string, std::array<float, _max_word_dim>> m_dict;
        std::vector<std::pair<std::string, std::string>> m_dataset;
        std::shared_ptr<sentencepiece::SentencePieceProcessor> m_spmodel = nullptr;
    };

    class LinearBnReluImpl : public torch::nn::Module {
    public:
        LinearBnReluImpl(int intput_features, int output_features);
        torch::Tensor forward(torch::Tensor x);
    private:
        //layers
        torch::nn::Linear ln{ nullptr };
        torch::nn::BatchNorm1d bn{nullptr};
    };
    TORCH_MODULE(LinearBnRelu);

    class ConvReluBnImpl : public torch::nn::Module {
    public:
        ConvReluBnImpl(int input_channel = 3, int output_channel = 64, int kernel_size = 3, int stride = 1);
        torch::Tensor forward(torch::Tensor x);
    private:
        inline torch::nn::Conv2dOptions conv_options(int64_t in_planes, int64_t out_planes, int64_t kerner_size,
            int64_t stride = 1, int64_t padding = 0, bool with_bias = false) {
            torch::nn::Conv2dOptions conv_options = torch::nn::Conv2dOptions(in_planes, out_planes, kerner_size);
            conv_options.stride(stride);
            conv_options.padding(padding);
            conv_options.bias(with_bias);
            return conv_options;
        }

        // Declare layers
        torch::nn::Conv2d conv{ nullptr };
        torch::nn::BatchNorm2d bn{ nullptr };
    };
    TORCH_MODULE(ConvReluBn);

    class MLP : public torch::nn::Module {
    public:
        MLP(int in_features, int out_features);
        torch::Tensor forward(torch::Tensor x);
    private:
        int mid_features[3] = { 32,64,128 };
        LinearBnRelu ln1{ nullptr };
        LinearBnRelu ln2{ nullptr };
        LinearBnRelu ln3{ nullptr };
        torch::nn::Linear out_ln{nullptr};
    };

    class plainCNN : public torch::nn::Module {
    public:
        plainCNN(int in_channels, int out_channels);
        torch::Tensor forward(torch::Tensor x);
    private:
        inline torch::nn::Conv2dOptions conv_options(int64_t in_planes, int64_t out_planes, int64_t kerner_size,
            int64_t stride = 1, int64_t padding = 0, bool with_bias = false) {
            torch::nn::Conv2dOptions conv_options = torch::nn::Conv2dOptions(in_planes, out_planes, kerner_size);
            conv_options.stride(stride);
            conv_options.padding(padding);
            conv_options.bias(with_bias);
            return conv_options;
        }

        int mid_channels[3] = { 32,64,128 };
        ConvReluBn conv1{ nullptr };
        ConvReluBn down1{ nullptr };
        ConvReluBn conv2{ nullptr };
        ConvReluBn down2{ nullptr };
        ConvReluBn conv3{ nullptr };
        ConvReluBn down3{ nullptr };
        torch::nn::Conv2d out_conv{nullptr};
    };

    class LSTM : public torch::nn::Module {
    public:
        LSTM(int in_features, int hidden_layer_size, int out_size, int num_layers, bool batch_first);
        torch::Tensor forward(torch::Tensor x);
    private:
        inline torch::nn::LSTMOptions lstmOption(int in_features, int hidden_layer_size, int num_layers, bool batch_first = false, bool bidirectional = false) {
            torch::nn::LSTMOptions lstmOption = torch::nn::LSTMOptions(in_features, hidden_layer_size);
            lstmOption.num_layers(num_layers).batch_first(batch_first).bidirectional(bidirectional);
            return lstmOption;
        }
        torch::nn::LSTM lstm{ nullptr };
        torch::nn::Linear ln{nullptr};
        std::tuple<torch::Tensor, torch::Tensor> hidden_cell;
    };
    
    class CGruModelImpl : public torch::nn::Module {
    private:
#define _invalid_accuracy float(-1)
#define _default_training_epochs 100
        static const  std::uint32_t _gru_model_magic;
        static const std::uint32_t _gru_model_version;
    public:
        CGruModelImpl(const std::string& path, const std::string& output, const std::string& modelbin, const std::string& dictbin_path, std::uint32_t epochs = 100, std::uint32_t ratio = 8, bool bcpu = true) {
            m_input = path;
            m_output = output;
            m_ratio = ratio;
            m_modelbin = modelbin;
            m_dictbin = dictbin_path;
            m_bcpu = bcpu;
            m_num_epochs = epochs;
            if (m_bcpu) {
                m_device = std::make_unique<torch::Device>(torch::kCPU);
            }
            else {
                m_device = std::make_unique<torch::Device>(torch::kCUDA);
            }
            if (!m_device) {
                return;
            }
            m_datasets = std::make_unique<CDataSets>(m_input, m_dictbin, ratio);
            if (!m_datasets || !m_datasets->IsValid()) {
                return;
            }
            std::uint32_t input_size = m_datasets->getWorddim();
            m_linearmodel = torch::nn::Linear(input_size, 5);//m_datasets->getLabelnum());
            m_grumodel = torch::nn::GRU(torch::nn::GRUOptions(input_size, input_size).num_layers(1));
            register_module("gru", m_grumodel);
            register_module("linear", m_linearmodel);
            to(*m_device);
            m_grumodel->to(*m_device);
            m_linearmodel->to(*m_device);
            m_valid = true;
        }
        ~CGruModelImpl() = default;

        bool train();
        bool test();
        bool predict(const std::string& input_path);
        //bool packmodel(const std::string& modelpath);
        bool packmodel(CGruModel& model);
        bool unpackmodel(CGruModel& model);
        void SetkParam(std::uint32_t k) {
            m_k = k;
        }
        void outputTestFesult() const {
            if (m_accuracy != _invalid_accuracy) {
                std::cout << "accuracy: " << std::fixed << std::setprecision(2) << m_accuracy << "%" << std::endl;
            }
            return;
        }
        void outputPredictResult() {
            if (m_predict_result.defined()) {
                auto results = m_predict_result[0] * 100;
                std::vector<std::pair<float, std::int32_t>> precision;
                for (int i = 0; i < results.size(0); ++i) {
                    precision.push_back(std::pair<float, std::int32_t>(results[i].item<float>(), i));
                }
                std::sort(precision.begin(), precision.end(),
                    [](const std::pair<float, std::int32_t>& a, const std::pair<float, std::int32_t>& b) {
                        return a.first > b.first;
                    });
                if (m_k == 0) {
                    std::cout << "predict class: " << m_datasets->getLabel(precision[0].second) << std::endl;
                }
                else {
                    for (int i = 0; i < m_k; i++) {
                        std::cout << "predict class: " << m_datasets->getLabel(precision[i].second) << "\t" << std::fixed << std::setprecision(2) << precision[i].first << "%" << std::endl;
                    }
                }
            }
            m_k = 0;
            return;
        }
    private:
        torch::Tensor forward(torch::Tensor input);
        bool loadtorchmodel(CGruModel& model, const std::string& modelbin);
        bool m_valid = false;
        bool m_bcpu = true;
        std::string m_input;
        std::string m_output;
        std::string m_modelbin;
        std::string m_dictbin;
        std::uint32_t m_ratio = 0;
        std::uint32_t m_k = 1;
        float m_accuracy = _invalid_accuracy;
        torch::Tensor m_predict_result;
        std::unique_ptr<torch::Device> m_device = nullptr;
        std::uint32_t m_num_epochs = _default_training_epochs;
        std::unique_ptr<CDataSets> m_datasets = nullptr;
        torch::nn::GRU m_grumodel{ nullptr };
        torch::nn::Linear m_linearmodel { nullptr };
    };
    TORCH_MODULE(CGruModel);

    class CLstmModelImpl : public torch::nn::Module {
    private:
#define _invalid_accuracy float(-1)
#define _default_training_epochs 100
        static const  std::uint32_t _lstm_model_magic;
        static const std::uint32_t _lstm_model_version;
    public:
        CLstmModelImpl(const std::string& path, const std::string& output, const std::string& modelbin, const std::string& dictbin_path, std::uint32_t epochs = 100, std::uint32_t ratio = 8, bool bcpu = true) {
            m_input = path;
            m_output = output;
            m_ratio = ratio;
            m_modelbin = modelbin;
            m_dictbin = dictbin_path;
            m_bcpu = bcpu;
            m_num_epochs = epochs;
            if (m_bcpu) {
                m_device = std::make_unique<torch::Device>(torch::kCPU);
            }
            else {
                m_device = std::make_unique<torch::Device>(torch::kCUDA);
            }
            if (!m_device) {
                return;
            }
            m_datasets = std::make_unique<CDataSets>(m_input, m_dictbin, ratio);
            if (!m_datasets || !m_datasets->IsValid()) {
                return;
            }
            std::uint32_t input_size = m_datasets->getWorddim();
            m_linearmodel = torch::nn::Linear(input_size, 5);//m_datasets->getLabelnum());
            m_lstmmodel = torch::nn::LSTM(torch::nn::LSTMOptions(input_size, input_size).num_layers(1));
            register_module("lstm", m_lstmmodel);
            register_module("linear", m_linearmodel);
            to(*m_device);
            m_lstmmodel->to(*m_device);
            m_linearmodel->to(*m_device);
            m_valid = true;
        }
        ~CLstmModelImpl() = default;

        bool train();
        bool test();
        bool predict(const std::string& input_path);
        bool packmodel(CLstmModel& model);
        bool unpackmodel(CLstmModel& model);
        void SetkParam(std::uint32_t k) {
            m_k = k;
        }
        void outputTestFesult() const {
            if (m_accuracy != _invalid_accuracy) {
                std::cout << "accuracy: " << std::fixed << std::setprecision(2) << m_accuracy << "%" << std::endl;
            }
            return;
        }
        void outputPredictResult() {
            if (m_predict_result.defined()) {
                auto results = m_predict_result[0] * 100;
                std::vector<std::pair<float, std::int32_t>> precision;
                for (int i = 0; i < results.size(0); ++i) {
                    precision.push_back(std::pair<float, std::int32_t>(results[i].item<float>(), i));
                }
                std::sort(precision.begin(), precision.end(),
                    [](const std::pair<float, std::int32_t>& a, const std::pair<float, std::int32_t>& b) {
                        return a.first > b.first;
                    });
                if (m_k == 0) {
                    std::cout << "predict class: " << m_datasets->getLabel(precision[0].second) << std::endl;
                }
                else {
                    for (int i = 0; i < m_k; i++) {
                        std::cout << "predict class: " << m_datasets->getLabel(precision[i].second) << "\t" << std::fixed << std::setprecision(2) << precision[i].first << "%" << std::endl;
                    }
                }
            }
            m_k = 0;
            return;
        }
    private:
        torch::Tensor forward(torch::Tensor input);
        bool loadtorchmodel(CLstmModel& model, const std::string& modelbin);
        bool m_valid = false;
        bool m_bcpu = true;
        std::string m_input;
        std::string m_output;
        std::string m_modelbin;
        std::string m_dictbin;
        std::uint32_t m_ratio = 0;
        std::uint32_t m_k = 1;
        float m_accuracy = _invalid_accuracy;
        torch::Tensor m_predict_result;
        std::unique_ptr<torch::Device> m_device = nullptr;
        std::uint32_t m_num_epochs = _default_training_epochs;
        std::unique_ptr<CDataSets> m_datasets = nullptr;
        torch::nn::LSTM m_lstmmodel{ nullptr };
        torch::nn::Linear m_linearmodel { nullptr };
    };
    TORCH_MODULE(CLstmModel);

    class CFastTextModel {
    public:
        using ft_submodel = enum {
            submodel_cbow,
            submodel_skipgram,
            submodel_supervised,
        };

        CFastTextModel(const std::string& path, const std::string& output, const std::string& modelbin, std::uint32_t epochs = 100, std::uint32_t ratio = 8, ft_submodel submodel = submodel_supervised) {
            m_input = path;
            m_output = output;
            m_ratio = ratio;
            m_epochs = epochs;
            m_subtype = submodel;
            m_modelbin = modelbin;
            m_ftmodel = std::make_unique<fasttext::FastText>();
        }
        ~CFastTextModel() = default;

        bool train();
        bool test();
        bool predict();
        bool IsValid() const {
            if (!m_ftmodel)
                return false;
            if (!m_input.length())
                return false;
            return true;
        }
        void SetkParam(std::uint32_t k) {
            m_k = k;
        }
    private:
        std::uint32_t m_k = 1;
        std::uint32_t m_epochs = 100;
        std::string m_input;
        std::string m_output;
        std::string m_modelbin;
        std::uint32_t m_ratio = 0;
        ft_submodel m_subtype;
        std::unique_ptr<fasttext::FastText> m_ftmodel = nullptr;
    };

    class CAlbertModel {
    private:
        static const  std::uint32_t _albert_model_magic;
        static const std::uint32_t _albert_model_version;
    public:
        CAlbertModel(const std::string& path, const std::string& output, const std::string& modelbin, bool bcpu = true) {
            m_input = path;
            m_output = output;
            m_modelbin = modelbin;
            m_bcpu = bcpu;
            if (m_bcpu) {
                m_device = std::make_unique<torch::Device>(torch::kCPU);
            }
            else {
                m_device = std::make_unique<torch::Device>(torch::kCUDA);
            }
            if (!m_device) {
                return;
            }
            m_datasets = std::make_unique<CDataSets>(m_input, "", 8);
            if (!m_datasets || !m_datasets->IsValid()) {
                return;
            }
            m_valid = true;
        }
        ~CAlbertModel() = default;

        bool train();
        bool test();
        bool predict(const std::string& inputpath);
        bool packmodel(const std::string& dictbin);
        bool unpackmodel();
        void SetkParam(std::uint32_t k) {
            m_k = k;
        }
        void outputPredictResult() {
            auto getLabelfunc = [](std::int32_t intlabel) ->std::string {
                static std::vector<std::string> labels = {
                    "__label_forticlient__",
                    "__label_cert_manage_tool__",
                    "__label_putty__",
                    "__label_unknown__",
                    "__label_teams__",
                };
                if (intlabel < labels.size()) {
                    return labels[intlabel];
                }
                return "__label_unknown__";
            };
            if (m_predict_result.defined()) {
                auto results = m_predict_result[0] * 100;
                std::vector<std::pair<float, std::int32_t>> precision;
                for (int i = 0; i < results.size(0); ++i) {
                    precision.push_back(std::pair<float, std::int32_t>(results[i].item<float>(), i));
                }
                std::sort(precision.begin(), precision.end(),
                    [](const std::pair<float, std::int32_t>& a, const std::pair<float, std::int32_t>& b) {
                        return a.first > b.first;
                    });
                if (m_k == 0) {
                    std::cout << "predict class: " << getLabelfunc(precision[0].second) << std::endl;
                }
                else {
                    for (int i = 0; i < m_k; i++) {
                        std::cout << "predict class: " << getLabelfunc(precision[i].second) << "\t" << std::fixed << std::setprecision(2) << precision[i].first << "%" << std::endl;
                    }
                }
            }
            m_k = 0;
            return;
        }
    private:
        torch::Tensor forward(torch::Tensor inputids, torch::Tensor attentionmask);
        bool m_valid = false;
        bool m_bcpu = true;
        std::uint32_t m_k = 0;
        std::string m_input;
        std::string m_output;
        std::string m_modelbin;
        std::unique_ptr<torch::Device> m_device = nullptr;
        torch::jit::script::Module m_model;
        std::unique_ptr<CDataSets> m_datasets = nullptr;
        torch::Tensor m_predict_result;
    };
} // namespace cchips
