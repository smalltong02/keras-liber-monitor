#include "resource.h"
#include "Classifier.h"
#include <re2/re2.h>

namespace cchips {

    const std::uint32_t CGruModelImpl::_gru_model_magic = 0x5F475255;
    const std::uint32_t CGruModelImpl::_gru_model_version = 1;
    const std::uint32_t CLstmModelImpl::_lstm_model_magic = 0x4C53544D;
    const std::uint32_t CLstmModelImpl::_lstm_model_version = 1;
    const  std::uint32_t CAlbertModel::_albert_model_magic = 0x62657274;
    const std::uint32_t CAlbertModel::_albert_model_version = 1;

    LinearBnReluImpl::LinearBnReluImpl(int in_features, int out_features) {
        ln = register_module("ln", torch::nn::Linear(torch::nn::LinearOptions(in_features, out_features)));
        bn = register_module("bn", torch::nn::BatchNorm1d(out_features));
    }

    torch::Tensor LinearBnReluImpl::forward(torch::Tensor x) {
        x = torch::relu(ln->forward(x));
        x = bn(x);
        return x;
    }

    ConvReluBnImpl::ConvReluBnImpl(int input_channel, int output_channel, int kernel_size, int stride) {
        conv = register_module("conv", torch::nn::Conv2d(conv_options(input_channel, output_channel, kernel_size, stride, kernel_size / 2)));
        bn = register_module("bn", torch::nn::BatchNorm2d(output_channel));

    }

    torch::Tensor ConvReluBnImpl::forward(torch::Tensor x) {
        x = torch::relu(conv->forward(x));
        x = bn(x);
        return x;
    }

    MLP::MLP(int in_features, int out_features) {
        ln1 = LinearBnRelu(in_features, mid_features[0]);
        ln2 = LinearBnRelu(mid_features[0], mid_features[1]);
        ln3 = LinearBnRelu(mid_features[1], mid_features[2]);
        out_ln = torch::nn::Linear(mid_features[2], out_features);

        ln1 = register_module("ln1", ln1);
        ln2 = register_module("ln2", ln2);
        ln3 = register_module("ln3", ln3);
        out_ln = register_module("out_ln", out_ln);
    }

    torch::Tensor MLP::forward(torch::Tensor x) {
        x = ln1->forward(x);
        x = ln2->forward(x);
        x = ln3->forward(x);
        x = out_ln->forward(x);
        return x;
    }

    plainCNN::plainCNN(int in_channels, int out_channels) {
        conv1 = ConvReluBn(in_channels, mid_channels[0], 3);
        down1 = ConvReluBn(mid_channels[0], mid_channels[0], 3, 2);
        conv2 = ConvReluBn(mid_channels[0], mid_channels[1], 3);
        down2 = ConvReluBn(mid_channels[1], mid_channels[1], 3, 2);
        conv3 = ConvReluBn(mid_channels[1], mid_channels[2], 3);
        down3 = ConvReluBn(mid_channels[2], mid_channels[2], 3, 2);
        out_conv = torch::nn::Conv2d(conv_options(mid_channels[2], out_channels, 3));

        conv1 = register_module("conv1", conv1);
        down1 = register_module("down1", down1);
        conv2 = register_module("conv2", conv2);
        down2 = register_module("down2", down2);
        conv3 = register_module("conv3", conv3);
        down3 = register_module("down3", down3);
        out_conv = register_module("out_conv", out_conv);
    }

    torch::Tensor plainCNN::forward(torch::Tensor x) {
        x = conv1->forward(x);
        x = down1->forward(x);
        x = conv2->forward(x);
        x = down2->forward(x);
        x = conv3->forward(x);
        x = down3->forward(x);
        x = out_conv->forward(x);
        return x;
    }

    LSTM::LSTM(int in_features, int hidden_layer_size, int out_size, int num_layers, bool batch_first) {
        lstm = torch::nn::LSTM(lstmOption(in_features, hidden_layer_size, num_layers, batch_first));
        ln = torch::nn::Linear(hidden_layer_size, out_size);

        lstm = register_module("lstm", lstm);
        ln = register_module("ln", ln);
    }

    torch::Tensor LSTM::forward(torch::Tensor x) {
        auto lstm_out = lstm->forward(x);
        auto predictions = ln->forward(std::get<0>(lstm_out));
        return predictions.select(1, -1);
    }

    CDataSets::CDataSets(const std::string& dataset_path, const std::string& dictbin_path, std::uint32_t ratio)
        : m_ratio(ratio)
    {
        if (fs::is_regular_file(dataset_path)) {
            if (!LoadDataset(dataset_path)) {
                return;
            }
        }
        m_dictpath = dictbin_path;
        m_valid = true;
        return;
    }

    bool CDataSets::splitDatasets(const std::string& path) {
        if (!IsValid()) {
            return false;
        }
        std::random_device rd;
        std::mt19937 rng(rd());
        std::shuffle(m_dataset.begin(), m_dataset.end(), rng);
        m_splitpos = m_dataset.size() * m_ratio / 10;
        return true;
    }

    bool CDataSets::getTrainSets(std::vector<std::pair<std::string, std::string>>& train) const {
        if (!IsValid()) {
            return false;
        }
        if (m_splitpos == 0) {
            return false;
        }
        if (m_splitpos < m_dataset.size()) {
            return false;
        }
        train.clear();
        for (int count = 0; count < m_splitpos; count++) {
            train.push_back(m_dataset[count]);
        }
        return true;
    }

    bool CDataSets::getTestSets(std::vector<std::pair<std::string, std::string>>& test) const {
        if (!IsValid()) {
            return false;
        }
        if (m_splitpos == 0) {
            return false;
        }
        if (m_splitpos < m_dataset.size()) {
            return false;
        }
        test.clear();
        for (int count = m_splitpos; count < m_dataset.size(); count++) {
            test.push_back(m_dataset[count]);
        }
        return true;
    }

    void CDataSets::bagging() {
        if (!IsValid()) {
            return;
        }
        if (m_splitpos == 0) {
            return;
        }
        if (m_splitpos < m_dataset.size()) {
            return;
        }
        std::random_device rd;
        std::mt19937 rng(rd());

        std::shuffle(m_dataset.begin(), m_dataset.end(), rng);
        return;
    }

    bool CDataSets::MakeDict(dict_type type) {
        if (!IsValid()) {
            return false;
        }
        if (m_dict.size()) {
            return true;
        }
        if (m_path.empty()) {
            return true;
        }
        if (!m_dictpath.empty()) {
            return true;
        }
        std::cout << "making dict..." << std::endl;
        switch (type) {
        case dict_word2vec:
        {
            if (MakeWord2vecDict()) {
                m_dicttype = dict_word2vec;
                return true;
            }
        }
        break;
        case dict_fasttext:
        {
            if (MakeFasttextDict()) {
                m_dicttype = dict_fasttext;
                return true;
            }
        }
        break;
        }
        return false;
    }

    bool CDataSets::MakeWord2vecDict() {
        auto tmppath = fs::temp_directory_path().append("_dict.bin");
        CWord2Vec word2vec;
        word2vec.Initialize(m_path.string(), tmppath.string(), {}, {});
        word2vec.TrainModel();
        word2vec.ClearAllCache();
        if (fs::is_regular_file(tmppath)) {
            m_dictpath = tmppath;
            return true;
        }
        return false;
    }

    bool CDataSets::MakeFasttextDict() {
        return false;
    }

    bool CDataSets::LoadDict(dict_type type) {
        if (!IsValid()) {
            return false;
        }
        if (type != dict_sentencepiece && m_dict.size()) {
            return true;
        }
        std::cout << "loading dict..." << std::endl;
        switch (type) {
        case dict_word2vec:
        {
            if (LoadWord2vecDict()) {
                m_dicttype = type;
                return true;
            }
        }
        break;
        case dict_fasttext:
        {
            if (LoadFasttextDict()) {
                m_dicttype = type;
                return true;
            }
        }
        break;
        case dict_sentencepiece:
        {
            if (LoadSentencepieceDict()) {
                m_dicttype = type;
                return true;
            }
        }
        break;
        }
        return false;
    }

    bool CDataSets::LoadSentencepieceDict() {
        if (!m_spmodel) {
            return false;
        }
        std::vector<std::string> pieces;
        auto status = m_spmodel->Encode("hello", &pieces);
        if (!status.ok()) {
            return false;
        }
        return true;
    }

    bool CDataSets::LoadWord2vecDict() {
        if (m_dictpath.empty()) {
            return false;
        }
        if (!fs::exists(m_dictpath)) {
            return false;
        }
        auto filesize = fs::file_size(m_dictpath);
        if (filesize == static_cast<uintmax_t>(-1) || filesize == 0) {
            return false;
        }
        if (filesize >= _max_file_size)
            return false;
        std::ifstream infile;
        infile.open(m_dictpath, std::ios::in | std::ios::binary);
        if (!infile.is_open()) {
            return false;
        }
        m_dict.clear();
        std::uint32_t linenum = 0;
        std::string line;
        std::string word;
        std::string fvec_str;
        std::array<float, _max_word_dim> fvec;
        try {
            while (std::getline(infile, line)) {
                std::stringstream iss(line);
                iss >> word;
                int index = 0;
                while (iss >> fvec_str) {
                    fvec[index++] = std::stof(fvec_str);
                    if (index >= m_word_dim) break;
                }
                if (fvec.size() != m_word_dim) {
                    throw std::runtime_error("err word dim.");
                }
                m_dict[word] = fvec;
            }
        }
        catch (const std::exception& e) {
            m_dict.clear();
            infile.close();
            return false;
        }
        infile.close();
        return true;
    }

    bool CDataSets::LoadFasttextDict() {
        return false;
    }
    std::unique_ptr<torch::Dict<std::string, torch::Tensor>> CDataSets::get_sp(size_t index) const {
        if (!IsValid()) {
            return {};
        }
        if (!m_spmodel) {
            return {};
        }
        if (index >= size()) {
            return {};
        }
        std::uint32_t label = findLabelidx(m_dataset[index].first);
        if (label == std::uint32_t(-1)) {
            return {};
        }
        re2::RE2 pattern("#(.*?)#");
        std::stringstream ss(m_dataset[index].second);
        std::string word;
        std::stringstream sequences;
        while (ss >> word) {
            if (!RE2::FullMatch(word, pattern)) {
                if (!sequences.str().length()) {
                    sequences << word;
                }
                else {
                    sequences << " " << word;
                }
            }
        }
        std::vector <std::string> pieces;
        auto status = m_spmodel->Encode(sequences.str(), &pieces);
        if (!status.ok()) {
            return {};
        }
        std::uint32_t max_ids = (pieces.size() > 512) ? 512 : pieces.size();
        torch::Tensor input_ids = torch::empty({1, (std::int32_t)max_ids }, torch::kLong);
        torch::Tensor attention_mask = torch::ones({1, (std::int32_t)max_ids }, torch::kFloat32);
        
        for (size_t i = 0; i < max_ids; ++i) {
            input_ids[0][i] = m_spmodel->PieceToId(pieces[i]);
        }
        //std::vector <std::string> pieces_dup;
        //for (size_t i = 0; i < max_ids; ++i) {
        //    pieces_dup.push_back(pieces[i]);
        //}
        //std::string detokenized;
        //m_spmodel->Decode(pieces_dup, &detokenized);
        std::unique_ptr<torch::Dict<std::string, torch::Tensor>> dict_ptr = std::make_unique<torch::Dict<std::string, torch::Tensor>>();
        if (!dict_ptr) {
            return {};
        }
        dict_ptr->insert("input_ids", input_ids);
        dict_ptr->insert("attention_mask", attention_mask);
        return std::move(dict_ptr);
    }
    torch::data::Example<> CDataSets::get(size_t index) {
        if (!IsValid()) {
            return {};
        }
        if (index >= size()) {
            return {};
        }
        std::vector<std::array<float, _max_word_dim>> sequences;
        std::uint32_t label = findLabelidx(m_dataset[index].first);
        if (label == std::uint32_t(-1)) {
            return {};
        }
        int word_counts = 0;
        std::stringstream ss(m_dataset[index].second);
        std::string word;
        while (ss >> word) {
            auto& diciter = m_dict.find(word);
            if (diciter != m_dict.end()) {
                sequences.push_back(diciter->second);
            }
            else {
                sequences.push_back(m_dict[SPECIAL_CORPUS_UNK]);
            }
            word_counts++;
            if (word_counts >= m_sequences_size) {
                break;
            }
        }
        int M = sequences.size();
        int N = _max_word_dim;
        torch::TensorOptions options(torch::kFloat32);
        torch::Tensor labeltor = torch::tensor({ (std::int32_t)label }, torch::kLong);
        torch::Tensor datator = torch::empty({ M, N }, options);
        for (int i = 0; i < M; i++) {
            datator[i] = torch::from_blob(sequences[i].data(), { N }, options);
        }
        //        std::cout << datator.sizes() << std::endl;
        //        std::cout << labeltor.sizes() << std::endl;
        return { datator, labeltor };

        //if (index * m_batchsize >= m_dataset.size()) {
        //    return {};
        //}
        //std::vector<std::uint32_t> label_vec;
        //std::vector<std::vector<std::array<float, _max_word_dim>>> sequences_vec;
        //int loop_size = ((index + 1) * m_batchsize) <= (m_dataset.size() - 1) ? ((index + 1) * m_batchsize) : (m_dataset.size() - 1);
        //for (int lab_count = index * m_batchsize; lab_count < loop_size; lab_count++) {
        //    auto labidx = findLabelidx(m_dataset[lab_count].first);
        //    if (labidx == std::uint32_t(-1)) {
        //        continue;
        //    }
        //    label_vec.push_back(labidx);
        //    int word_counts = 0;
        //    std::vector<std::array<float, _max_word_dim>> word_vec;
        //    std::stringstream ss(m_dataset[lab_count].second);
        //    std::string word;
        //    while(ss >> word) {
        //        auto& diciter = m_dict.find(word);
        //        if (diciter != m_dict.end()) {
        //            word_vec.push_back(diciter->second);
        //        }
        //        else {
        //            word_vec.push_back(m_dict[SPECIAL_CORPUS_UNK]);
        //        }
        //        word_counts++;
        //    }
        //    for (; word_counts < m_sequences_size; word_counts++) {
        //        word_vec.push_back(m_dict[SPECIAL_CORPUS_UNK]);
        //    }
        //    sequences_vec.push_back(word_vec);
        //}
        //int N = label_vec.size();
        //int O = _max_word_dim;
        //torch::TensorOptions options(torch::kFloat32);
        //torch::Tensor label = torch::empty({ N }, torch::kLong);
        //for (int i = 0; i < N; i++) {
        //    label[i] = (long)(label_vec[i]);
        //}
        //int M = sequences_vec.size();
        //N = sequences_vec[0].size();
        //torch::Tensor data = torch::empty({ M, N, O }, options);
        //for (int i = 0; i < M; i++) {
        //    for (int j = 0; j < N; j++) {
        //        data[i][j] = torch::from_blob(sequences_vec[i][j].data(), { O }, options);
        //    }
        //}
        //data = torch::transpose(data, 0, 1);
        ////torch::Tensor label = torch::from_blob(const_cast<char*>(m_dataset[index].first.c_str()), { (int)m_dataset[index].first.length() }, torch::kByte);
        ////torch::Tensor data = torch::from_blob(const_cast<char*>(m_dataset[index].second.c_str()), { (int)m_dataset[index].second.length() }, torch::kByte);
        //return { data, label };
    }
    torch::optional<size_t> CDataSets::size() const {
        return m_dataset.size();
    }
    std::tuple<std::tuple<torch::Tensor, torch::Tensor>, torch::Tensor> CDataSets::LoadBatch(const std::vector<torch::data::Example<>>& batch) const {
        if (!batch.size()) {
            return {};
        }
        std::vector<torch::Tensor> data_batch;
        std::vector<torch::Tensor> target_batch;
        auto batchsize = batch.size();
        auto expand = m_sequences_size;
        std::vector<std::int32_t> lengths_vec;
        for (const auto& bat : batch) {
            int64_t dim_size = bat.data.size(0);
            lengths_vec.push_back(dim_size);
            //torch::Tensor randomsor = torch::zeros({ expand - dim_size, bat.data.size(1)});
            ////int elements_to_copy = std::min(randomsor.size(0), bat.data.size(0));
            ////randomsor.slice(0, 0, elements_to_copy).copy_(bat.data.slice(0, 0, elements_to_copy));
            //std::cout << randomsor.sizes() << std::endl;
            //std::vector<torch::Tensor> tensors_to_cat = { bat.data, randomsor };
            //torch::Tensor expanded_data = torch::cat(tensors_to_cat, 0);
            //std::cout << expanded_data.sizes() << std::endl;
            data_batch.push_back(bat.data/*expanded_data*/);
            target_batch.push_back(bat.target);
        }
        torch::Tensor lengths = torch::tensor(lengths_vec, torch::kLong);
        torch::Tensor data = torch::stack(data_batch);
        torch::Tensor target = torch::cat(target_batch);
        data = torch::transpose(data, 0, 1);
        //std::cout << data.sizes() << std::endl;
        //std::cout << target.sizes() << std::endl;
        return { {data, lengths}, target };
    }

    std::tuple<torch::Tensor, torch::Tensor> CDataSets::LoadSample(const std::string& input) {
        if (!input.length()) {
            return {};
        }
        initLabelVec();
        m_dataset.clear();
        m_dataset.push_back(std::pair(getLabel(0), input));
        std::cout << "LoadDataset: " << 1 << std::endl;
        if (m_dicttype == dict_sentencepiece) {
            auto dict_ptr = get_sp(0);
            if (!dict_ptr) {
                return {};
            }
            auto iter = dict_ptr->find("input_ids");
            if (iter == dict_ptr->end()) {
                return {};
            }
            torch::Tensor inputids = iter->value();
            iter = dict_ptr->find("attention_mask");
            if (iter == dict_ptr->end()) {
                return {};
            }
            torch::Tensor attentionmask = iter->value();
            return { inputids, attentionmask };
        }
        else {
            auto sample = get(0);
            if (!sample.data.defined()) {
                return {};
            }
            std::vector<torch::Tensor> data_batch;
            std::vector<std::int32_t> lengths_vec;
            int64_t dim_size = sample.data.size(0);
            lengths_vec.push_back(dim_size);
            data_batch.push_back(sample.data);
            torch::Tensor lengths = torch::tensor(lengths_vec, torch::kLong);
            torch::Tensor data = torch::stack(data_batch);
            data = torch::transpose(data, 0, 1);
            return { data, lengths };
        }
        return {};
    }

    bool CDataSets::save(std::ofstream& outfile) {
        if (!outfile.is_open()) {
            return false;
        }
        initLabelVec();
        outfile.write((char*)&(m_dicttype), sizeof(std::int32_t));
        outfile.write((char*)&(m_word_dim), sizeof(std::int32_t));
        outfile.write((char*)&(m_sequences_size), sizeof(std::int32_t));
        outfile.write((char*)&(m_batchsize), sizeof(std::int32_t));
        std::uint32_t labelsize = m_label.size();
        outfile.write((char*)&(labelsize), sizeof(std::int32_t));
        std::uint32_t wordnums = m_dict.size();
        outfile.write((char*)&(wordnums), sizeof(std::int32_t));
        for (auto& label : m_label) {
            outfile.write(label.data(), label.size());
            outfile.put(0);
        }
        for (auto& it : m_dict) {
            outfile.write(it.first.data(), it.first.size());
            outfile.put(0);
            outfile.write((char*)it.second.data(), (it.second.size() * sizeof(float)));
        }
        return true;
    }

    bool CDataSets::load(std::ifstream& infile) {
        auto getstr_func = [&](std::ifstream& infile) ->std::string {
            char c;
            std::string str;
            while ((c = infile.get()) != 0) {
                str += c;
            }
            return str;
        };
        auto getarray_func = [&](std::ifstream& infile)->std::array<float, _max_word_dim> {
            float flt = 0.0;
            std::array<float, _max_word_dim> word_array;
            infile.read((char*)word_array.data(), (word_array.size() * sizeof(float)));
            return word_array;
        };

        if (!infile.is_open()) {
            return false;
        }
        infile.read((char*)&(m_dicttype), sizeof(std::int32_t));
        infile.read((char*)&(m_word_dim), sizeof(std::int32_t));
        infile.read((char*)&(m_sequences_size), sizeof(std::int32_t));
        infile.read((char*)&(m_batchsize), sizeof(std::int32_t));
        std::uint32_t labelsize = 0;
        infile.read((char*)&(labelsize), sizeof(std::int32_t));
        std::uint32_t wordnums = 0;
        infile.read((char*)&(wordnums), sizeof(std::int32_t));
        for (int count = 0; count < labelsize; count++) {
            std::string str = getstr_func(infile);
            m_label.insert(str);
        }
        for (int count = 0; count < wordnums; count++) {
            std::string str = getstr_func(infile);
            auto word_array = getarray_func(infile);
            m_dict[str] = word_array;
        }
        return true;
    }

    bool CDataSets::loadsentencemodel(std::ifstream& infile, std::int32_t filesize) {
        if (!infile.is_open() || filesize == 0) {
            return false;
        }
        std::uint32_t fileoffset = infile.tellg();
        auto tmppath = fs::temp_directory_path().append("_dict.model");
        std::ofstream outfile(tmppath, std::ios::out | std::ios::binary);
        if (!outfile.is_open()) {
            return false;
        }
        std::vector<BYTE> buffer;
        buffer.resize(filesize);
        if (buffer.size() != filesize) {
            return false;
        }
        infile.read((char*)&buffer[0], filesize);
        std::uint32_t readed = infile.tellg();
        if (readed != (fileoffset + filesize)) {
            return false;
        }
        outfile.write((char*)buffer.data(), buffer.size());
        outfile.close();
        m_spmodel = std::make_shared<sentencepiece::SentencePieceProcessor>();
        auto status = m_spmodel->Load(tmppath.string());
        if (!status.ok()) {
            return false;
        }
        fs::remove(tmppath);
        return true;
    }

    bool CFastTextModel::train() {
        CFunduration func_duration("traning time: ");
        if (!IsValid()) {
            return false;
        }
        if (!m_output.length()) {
            return false;
        }
        if (m_ratio != 8 && m_ratio != 6) {
            return false;
        }
        std::ofstream ofs(m_output);
        if (!ofs.is_open()) {
            return false;
        }
        ofs.close();
        if (!fs::is_regular_file(m_input)) {
            return false;
        }
        fasttext::Args args = fasttext::Args();

        args.input = m_input;
        switch (m_subtype) {
        case submodel_cbow:
        {
            args.model = fasttext::model_name::cbow;
        }
        break;
        case submodel_skipgram:
            break;
        case submodel_supervised:
        default:
        {
            args.bucket = 0;
            args.model = fasttext::model_name::sup;
            args.loss = fasttext::loss_name::softmax;
            args.minCount = 1;
            args.minn = 0;
            args.maxn = 0;
            args.lr = 0.5;
            args.epoch = m_epochs;
            args.label = "__label_";
        }
        break;
        }
        try {
            m_ftmodel->train(args);
            m_ftmodel->saveModel(m_output);
        }
        catch (const std::exception& e) {
            std::cout << "Exception occurred during Fasttext model training: " << e.what() << std::endl;
            return false;
        }
        return true;
    }

    bool CFastTextModel::test() {
        CFunduration func_duration("testing time: ");
        return false;
    }

    bool CFastTextModel::predict() {
        CFunduration func_duration("predicting time: ");
        if (!IsValid()) {
            return false;
        }
        if (!m_modelbin.length()) {
            return false;
        }
        if (!fs::is_regular_file(m_input)) {
            return false;
        }
        if (!fs::is_regular_file(m_modelbin)) {
            return false;
        }
        try {
            std::unique_ptr<cchips::CJsonOptions> options = std::make_unique<cchips::CJsonOptions>("CFGRES", IDR_JSONPE_CFG);
            if (!options || !options->Parse()) {
                std::cout << "load config error." << std::endl;
                return false;
            }
            auto& staticmanager = cchips::GetCStaticFileManager();
            if (!staticmanager.Initialize(std::move(options))) {
                std::cout << "Initialize failed!" << std::endl;
                return false;
            }
            std::string output;
            if (!staticmanager.Scan(m_input, output, false) || !output.length()) {
                std::cout << "scan: " << m_input << " failed" << std::endl;
                return false;
            }
            auto& corpusmanager = cchips::GetCTextCorpusManager().GetInstance();
            corpusmanager.Initialize(output);
            if (!corpusmanager.GeneratingModelDatasets("fasttext", output, "")) {
                std::cout << "generating fasttext corpus failed!" << std::endl;
                return false;
            }
            if (!output.length()) {
                return false;
            }
            std::stringstream in;
            in << output;
            m_ftmodel->loadModel(m_modelbin);
            fasttext::real threshold = 0.0;
            std::vector<std::pair<fasttext::real, std::string>> predictions;
            if (m_k == 0) {
                m_ftmodel->predictLine(in, predictions, 1, threshold);
            }
            else {
                m_ftmodel->predictLine(in, predictions, m_k, threshold);
            }
            if (predictions.size()) {
                if (m_k == 0) {
                    std::cout << "predict class: " << predictions[0].second << std::endl;
                }
                else {
                    for (auto pre : predictions) {
                        std::cout << "predict class: " << pre.second << " similarity: " << pre.first << std::endl;
                    }
                }
            }
            else {
                std::cout << "predict failed!" << std::endl;
            }
            m_k = 0;
            return true;
        }
        catch (const std::exception& e) {
            std::cout << "Exception occurred during Fasttext model predicting: " << e.what() << std::endl;
            return false;
        }
    }

    torch::Tensor CGruModelImpl::forward(torch::Tensor input)
    {
        if (!m_valid) {
            return {};
        }
        //auto packed_input = torch::nn::utils::rnn::pack_padded_sequence(input, lengths, false, false);
        torch::Tensor tensor_output, tensor_h0_n;
        //std::cout << input.sizes() << std::endl;
        std::tie(tensor_output, tensor_h0_n) = m_grumodel->forward(input);
        //std::cout << packed_input.batch_sizes() << std::endl;
        //gru_output = torch::nn::utils::rnn::pad_packed_sequence(gru_output, false, 0)[0];
        //torch::Tensor indices = lengths.to(torch::kLong);
        //indices = indices.cuda();
        torch::Tensor last_output = tensor_output[-1];
        tensor_output = torch::transpose(tensor_output, 0, 1);
        //std::cout << last_output << std::endl;
        //std::vector<torch::Tensor> tensor_vec;
        //for (int index = 0; index < indices.size(0); index++) {
        //    tensor_vec.push_back(gru_output[index][6]/*gru_output[index][indices[index]]*/);
        //    std::cout << gru_output[index][6] << std::endl;
        //}
        //torch::Tensor last_output = torch::stack(gru_output);
        //std::cout << last_output.sizes() << std::endl;
        //torch::Tensor last_output = torch::mean(gru_output, 0);
        //std::cout << last_output << std::endl;
        torch::Tensor output = m_linearmodel->forward(last_output);
        return output;
    }

    bool CGruModelImpl::train() {
        CFunduration func_duration("traning time: ");
        if (!m_valid) {
            return false;
        }
        if (!m_datasets->MakeDict()) {
            return false;
        }
        m_datasets->LoadDict();
        std::cout << "GRU Model training start... " << std::endl;
        m_grumodel->train();
        m_linearmodel->train();
        torch::nn::CrossEntropyLoss criterion;
        torch::optim::SGD optimizer(parameters(), torch::optim::SGDOptions(0.01));
        int step_size = 10;
        float gamma = 0.1;
        torch::optim::StepLR scheduler(optimizer, step_size, gamma);
        try {
            for (int epoch = 0; epoch < m_num_epochs; ++epoch) {
                auto data_loader = torch::data::make_data_loader(
                    *m_datasets,
                    torch::data::DataLoaderOptions().batch_size(1/*m_datasets->getBatchsize()*/).workers(10));
                std::uint32_t num_samples = 0;
                std::uint32_t num_batches = 0;
                std::float_t average_loss = 0.0;
                std::float_t total_loss = 0.0;
                for (const auto& batch : *data_loader)
                {
                    torch::Tensor inputs, lengths, targets;
                    std::tie(std::tie(inputs, lengths), targets) = m_datasets->LoadBatch(batch);
                    if (inputs.defined() && targets.defined()) {
                        inputs = inputs.to(*m_device);
                        targets = targets.to(*m_device);
                        //std::cout << inputs.sizes() << std::endl;
                        //std::cout << targets.sizes() << std::endl;

                        auto outputs = forward(inputs);
                        //std::cout << outputs << std::endl;
                        //std::cout << targets << std::endl;
                        outputs = outputs.to(*m_device);
                        auto loss = criterion(outputs, targets);
                        optimizer.zero_grad();
                        loss.backward();
                        optimizer.step();
                        total_loss += loss.item<float>();
                        num_batches++;
                        //num_samples += m_datasets->getBatchsize();
                        //std::cout << "internel Epoch: " << epoch + 1 << ", Loss: " << loss.item<float>()/* << "  num_samples: " << num_samples*/ << std::endl;
                    }
                    else {
                        break;
                    }
                }
                average_loss = total_loss / num_batches;
                std::cout << "Epoch: " << epoch + 1 << ", average loss: " << average_loss << std::endl;
            }
        }
        catch (const std::exception& e) {
            std::cout << "Exception occurred during model training: " << e.what() << std::endl;
        }
        std::cout << "Model training end... " << std::endl;
        if (m_output.length()) {
            //std::cout << "save Model..." << std::endl;
            //torch::serialize::OutputArchive output_archive;
            //save(output_archive);
            //output_archive.save_to(m_output);
        }
        return true;
    }

    bool CGruModelImpl::test() {
        CFunduration func_duration("testing time: ");
        if (!m_valid) {
            return false;
        }
        m_datasets->LoadDict();
        std::cout << "GRU Model testing start... " << std::endl;
        m_grumodel->eval();
        m_linearmodel->eval();
        try {
            std::atomic_uint32_t corrects = 0;
            torch::NoGradGuard no_grad;
            auto data_loader = torch::data::make_data_loader(
                *m_datasets,
                torch::data::DataLoaderOptions().batch_size(1).workers(10));
            for (const auto& batch : *data_loader)
            {
                torch::Tensor inputs, lengths, targets;
                std::tie(std::tie(inputs, lengths), targets) = m_datasets->LoadBatch(batch);
                if (inputs.defined() && targets.defined()) {
                    inputs = inputs.to(*m_device);
                    targets = targets.to(*m_device);
                    auto outputs = forward(inputs);
                    outputs = outputs.to(*m_device);
                    //auto probabilities = torch::softmax(outputs, -1);
                    torch::Tensor predictions = torch::argmax(outputs, 1);
                    //std::cout << outputs << std::endl;
                    auto t1 = predictions[0].item<int>();
                    auto t2 = targets[0].item<int>();
                    //std::cout << probabilities << std::endl;
                    //auto results = probabilities[0] * 100;
                    //for (int i = 0; i < results.size(0); ++i) {
                        //std::cout << std::fixed << std::setprecision(2) << results[i].item<float>() << "%" << " ";
                    //}
                    //std::cout << std::endl;
                    if (t1 == t2) {
                        //std::cout << t1 << " " << t2 << std::endl;
                        corrects++;
                    }
                }
            }
            m_accuracy = ((float)corrects / (float)m_datasets->size().value()) * 100;
        }
        catch (const std::exception& e) {
            std::cout << "Exception occurred during model testing: " << e.what() << std::endl;
            return false;
        }
        outputTestFesult();
        return true;
    }

    bool CGruModelImpl::predict(const std::string& input_path) {
        CFunduration func_duration("predicting time: ");
        if (!m_valid) {
            return false;
        }
        m_datasets->LoadDict();
        std::cout << "GRU Model predicting start... " << std::endl;
        m_grumodel->to(*m_device);
        m_linearmodel->to(*m_device);
        m_grumodel->eval();
        m_linearmodel->eval();
        m_predict_result = torch::Tensor();
        try {
            std::unique_ptr<cchips::CJsonOptions> options = std::make_unique<cchips::CJsonOptions>("CFGRES", IDR_JSONPE_CFG);
            if (!options || !options->Parse()) {
                std::cout << "load config error." << std::endl;
                return false;
            }
            auto& staticmanager = cchips::GetCStaticFileManager();
            if (!staticmanager.Initialize(std::move(options))) {
                std::cout << "Initialize failed!" << std::endl;
                return false;
            }
            std::string output;
            if (!staticmanager.Scan(input_path, output, false) || !output.length()) {
                std::cout << "scan: " << input_path << " failed" << std::endl;
                return false;
            }
            auto& corpusmanager = cchips::GetCTextCorpusManager().GetInstance();
            corpusmanager.Initialize(output);
            if (!corpusmanager.GeneratingModelDatasets("fasttext", output, "")) {
                std::cout << "generating fasttext corpus failed!" << std::endl;
                return false;
            }
            if (!output.length()) {
                return false;
            }

            std::atomic_uint32_t corrects = 0;
            torch::NoGradGuard no_grad;
            torch::Tensor inputs, lengths;
            std::tie(inputs, lengths) = m_datasets->LoadSample(output);
            if (inputs.defined()) {
                inputs = inputs.to(*m_device);
                auto outputs = forward(inputs);
                outputs = outputs.to(*m_device);
                m_predict_result = torch::softmax(outputs, -1);
                //auto t1 = predictions[0].item<int>();
                //std::cout << "predict: " << t1 << " label: " << m_datasets->getLabel(t1) << std::endl;
            }
        }
        catch (const std::exception& e) {
            std::cout << "Exception occurred during model predicting: " << e.what() << std::endl;
            return false;
        }
        outputPredictResult();
        return true;
    }

    bool CGruModelImpl::packmodel(CGruModel& model) {
        if (!m_valid) {
            return false;
        }
        std::cout << "save gru model..." << std::endl;
        torch::save(model, m_output);
        if (m_output.empty()) {
            return false;
        }
        if (!m_datasets || !m_datasets->IsValid()) {
            return false;
        }
        if (!fs::is_regular_file(m_output)) {
            return false;
        }
        std::uint32_t filesize = fs::file_size(m_output);
        if (filesize == static_cast<std::uint32_t>(-1) || filesize == 0) {
            return false;
        }
        std::ifstream infile;
        infile.open(m_output, std::ios::in | std::ios::binary);
        if (!infile.is_open()) {
            return false;
        }
        std::vector<BYTE> buffer;
        buffer.resize(filesize);
        if (buffer.size() != filesize) {
            return false;
        }
        infile.seekg(0, std::ios::beg);
        infile.read((char*)&buffer[0], filesize);
        auto readed = infile.tellg();
        if (readed != filesize) {
            return false;
        }
        infile.close();
        std::ofstream outfile;
        outfile.open(m_output, std::ios::out | std::ios::binary);
        if (!outfile.is_open()) {
            return false;
        }
        outfile.write((char*)&(_gru_model_magic), sizeof(std::int32_t));
        outfile.write((char*)&(_gru_model_version), sizeof(std::int32_t));
        outfile.write((char*)&(filesize), sizeof(std::int32_t));
        outfile.write((char*)buffer.data(), buffer.size());
        if (m_datasets->save(outfile)) {
            std::cout << "save success..." << std::endl;
            return true;
        }
        std::cout << "save failed..." << std::endl;
        return false;
    }

    bool CGruModelImpl::unpackmodel(CGruModel& model) {
        if (!m_valid) {
            return false;
        }
        if (m_modelbin.empty()) {
            return false;
        }
        if (!fs::is_regular_file(m_modelbin)) {
            return false;
        }
        std::uint32_t filesize = fs::file_size(m_modelbin);
        if (filesize == static_cast<std::uint32_t>(-1) || filesize == 0) {
            return false;
        }
        if (filesize < 12) {
            return false;
        }
        std::ifstream infile;
        infile.open(m_modelbin, std::ios::in | std::ios::binary);
        if (!infile.is_open()) {
            return false;
        }
        std::int32_t gru_magic = 0;
        std::int32_t gru_version = 0;
        infile.read((char*)&(gru_magic), sizeof(std::int32_t));
        infile.read((char*)&(gru_version), sizeof(std::int32_t));
        if (gru_magic != _gru_model_magic || gru_version != _gru_model_version) {
            infile.close();
            return loadtorchmodel(model, m_modelbin);
        }
        std::int32_t model_size = 0;
        infile.read((char*)&(model_size), sizeof(std::int32_t));
        if (model_size == 0 || filesize < 12 + model_size) {
            return false;
        }
        auto tmppath = fs::temp_directory_path().append("_model.pt");
        std::ofstream outfile(tmppath, std::ios::out | std::ios::binary);
        if (!outfile.is_open()) {
            return false;
        }
        std::vector<BYTE> buffer;
        buffer.resize(model_size);
        if (buffer.size() != model_size) {
            return false;
        }
        infile.read((char*)&buffer[0], model_size);
        auto readed = infile.tellg();
        if (readed != 12 + model_size) {
            return false;
        }
        outfile.write((char*)buffer.data(), buffer.size());
        outfile.close();
        loadtorchmodel(model, tmppath.string());
        fs::remove(tmppath);
        return m_datasets->load(infile);
    }

    bool CGruModelImpl::loadtorchmodel(CGruModel& model, const std::string& modelbin) {
        torch::load(model, modelbin);
        return true;
    }

    torch::Tensor CLstmModelImpl::forward(torch::Tensor input)
    {
        if (!m_valid) {
            return {};
        }
        //auto packed_input = torch::nn::utils::rnn::pack_padded_sequence(input, lengths, false, false);
        torch::Tensor tensor_output, tensor_h0_n, tensor_hh_n;
        //std::cout << input.sizes() << std::endl;
        std::tie(tensor_output, std::tie(tensor_h0_n, tensor_hh_n)) = m_lstmmodel->forward(input);
        //std::cout << packed_input.batch_sizes() << std::endl;
        //gru_output = torch::nn::utils::rnn::pad_packed_sequence(gru_output, false, 0)[0];
        //torch::Tensor indices = lengths.to(torch::kLong);
        //indices = indices.cuda();
        torch::Tensor last_output = tensor_output[-1];
        tensor_output = torch::transpose(tensor_output, 0, 1);
        //std::cout << last_output << std::endl;
        //std::vector<torch::Tensor> tensor_vec;
        //for (int index = 0; index < indices.size(0); index++) {
        //    tensor_vec.push_back(gru_output[index][6]/*gru_output[index][indices[index]]*/);
        //    std::cout << gru_output[index][6] << std::endl;
        //}
        //torch::Tensor last_output = torch::stack(gru_output);
        //std::cout << last_output.sizes() << std::endl;
        //torch::Tensor last_output = torch::mean(gru_output, 0);
        //std::cout << last_output << std::endl;
        torch::Tensor output = m_linearmodel->forward(last_output);
        return output;
    }

    bool CLstmModelImpl::train() {
        CFunduration func_duration("training time: ");
        if (!m_valid) {
            return false;
        }
        if (!m_datasets->MakeDict()) {
            return false;
        }
        m_datasets->LoadDict();
        std::cout << "LSTM Model training start... " << std::endl;
        m_lstmmodel->train();
        m_linearmodel->train();
        torch::nn::CrossEntropyLoss criterion;
        torch::optim::SGD optimizer(parameters(), torch::optim::SGDOptions(0.01));
        int step_size = 10;
        float gamma = 0.1;
        torch::optim::StepLR scheduler(optimizer, step_size, gamma);
        try {
            for (int epoch = 0; epoch < m_num_epochs; ++epoch) {
                auto data_loader = torch::data::make_data_loader(
                    *m_datasets,
                    torch::data::DataLoaderOptions().batch_size(1/*m_datasets->getBatchsize()*/).workers(10));
                std::uint32_t num_samples = 0;
                std::uint32_t num_batches = 0;
                std::float_t average_loss = 0.0;
                std::float_t total_loss = 0.0;
                for (const auto& batch : *data_loader)
                {
                    torch::Tensor inputs, lengths, targets;
                    std::tie(std::tie(inputs, lengths), targets) = m_datasets->LoadBatch(batch);
                    if (inputs.defined() && targets.defined()) {
                        inputs = inputs.to(*m_device);
                        targets = targets.to(*m_device);
                        //std::cout << inputs.sizes() << std::endl;
                        //std::cout << targets.sizes() << std::endl;

                        auto outputs = forward(inputs);
                        //std::cout << outputs << std::endl;
                        //std::cout << targets << std::endl;
                        auto loss = criterion(outputs, targets);
                        optimizer.zero_grad();
                        loss.backward();
                        optimizer.step();
                        total_loss += loss.item<float>();
                        num_batches++;
                        //num_samples += m_datasets->getBatchsize();
                        //std::cout << "internel Epoch: " << epoch + 1 << ", Loss: " << loss.item<float>()/* << "  num_samples: " << num_samples*/ << std::endl;
                    }
                    else {
                        break;
                    }
                }
                average_loss = total_loss / num_batches;
                std::cout << "Epoch: " << epoch + 1 << ", average loss: " << average_loss << std::endl;
            }
        }
        catch (const std::exception& e) {
            std::cout << "Exception occurred during model training: " << e.what() << std::endl;
        }
        std::cout << "Model training end... " << std::endl;
        if (m_output.length()) {
            //std::cout << "save Model..." << std::endl;
            //torch::serialize::OutputArchive output_archive;
            //save(output_archive);
            //output_archive.save_to(m_output);
        }
        return true;
    }

    bool CLstmModelImpl::test() {
        CFunduration func_duration("testing time: ");
        if (!m_valid) {
            return false;
        }
        m_datasets->LoadDict();
        std::cout << "LSTM Model testing start... " << std::endl;
        m_lstmmodel->eval();
        m_linearmodel->eval();
        try {
            std::atomic_uint32_t corrects = 0;
            torch::NoGradGuard no_grad;
            auto data_loader = torch::data::make_data_loader(
                *m_datasets,
                torch::data::DataLoaderOptions().batch_size(1).workers(10));
            for (const auto& batch : *data_loader)
            {
                torch::Tensor inputs, lengths, targets;
                std::tie(std::tie(inputs, lengths), targets) = m_datasets->LoadBatch(batch);
                if (inputs.defined() && targets.defined()) {
                    inputs = inputs.to(*m_device);
                    targets = targets.to(*m_device);
                    auto outputs = forward(inputs);
                    //auto probabilities = torch::softmax(outputs, -1);
                    torch::Tensor predictions = torch::argmax(outputs, 1);
                    //std::cout << outputs << std::endl;
                    auto t1 = predictions[0].item<int>();
                    auto t2 = targets[0].item<int>();
                    //std::cout << probabilities << std::endl;
                    //auto results = probabilities[0] * 100;
                    //for (int i = 0; i < results.size(0); ++i) {
                        //std::cout << std::fixed << std::setprecision(2) << results[i].item<float>() << "%" << " ";
                    //}
                    //std::cout << std::endl;
                    if (t1 == t2) {
                        //std::cout << t1 << " " << t2 << std::endl;
                        corrects++;
                    }
                }
            }
            m_accuracy = ((float)corrects / (float)m_datasets->size().value()) * 100;
        }
        catch (const std::exception& e) {
            std::cout << "Exception occurred during model testing: " << e.what() << std::endl;
            return false;
        }
        outputTestFesult();
        return true;
    }

    bool CLstmModelImpl::predict(const std::string& input_path) {
        CFunduration func_duration("predicting time: ");
        if (!m_valid) {
            return false;
        }
        m_datasets->LoadDict();
        std::cout << "LSTM Model predicting start... " << std::endl;
        m_lstmmodel->to(*m_device);
        m_linearmodel->to(*m_device);
        m_lstmmodel->eval();
        m_linearmodel->eval();
        m_predict_result = torch::Tensor();
        try {
            std::unique_ptr<cchips::CJsonOptions> options = std::make_unique<cchips::CJsonOptions>("CFGRES", IDR_JSONPE_CFG);
            if (!options || !options->Parse()) {
                std::cout << "load config error." << std::endl;
                return false;
            }
            auto& staticmanager = cchips::GetCStaticFileManager();
            if (!staticmanager.Initialize(std::move(options))) {
                std::cout << "Initialize failed!" << std::endl;
                return false;
            }
            std::string output;
            if (!staticmanager.Scan(input_path, output, false) || !output.length()) {
                std::cout << "scan: " << input_path << " failed" << std::endl;
                return false;
            }
            auto& corpusmanager = cchips::GetCTextCorpusManager().GetInstance();
            corpusmanager.Initialize(output);
            if (!corpusmanager.GeneratingModelDatasets("fasttext", output, "")) {
                std::cout << "generating fasttext corpus failed!" << std::endl;
                return false;
            }
            if (!output.length()) {
                return false;
            }

            std::atomic_uint32_t corrects = 0;
            torch::NoGradGuard no_grad;
            torch::Tensor inputs, lengths;
            std::tie(inputs, lengths) = m_datasets->LoadSample(output);
            if (inputs.defined()) {
                inputs = inputs.to(*m_device);
                auto outputs = forward(inputs);
                m_predict_result = torch::softmax(outputs, -1);
                //auto t1 = predictions[0].item<int>();
                //std::cout << "predict: " << t1 << " label: " << m_datasets->getLabel(t1) << std::endl;
            }
        }
        catch (const std::exception& e) {
            std::cout << "Exception occurred during model predicting: " << e.what() << std::endl;
            return false;
        }
        outputPredictResult();
        return true;
    }

    bool CLstmModelImpl::packmodel(CLstmModel& model) {
        if (!m_valid) {
            return false;
        }
        std::cout << "save lstm model..." << std::endl;
        torch::save(model, m_output);
        if (m_output.empty()) {
            return false;
        }
        if (!m_datasets || !m_datasets->IsValid()) {
            return false;
        }
        if (!fs::is_regular_file(m_output)) {
            return false;
        }
        std::uint32_t filesize = fs::file_size(m_output);
        if (filesize == static_cast<std::uint32_t>(-1) || filesize == 0) {
            return false;
        }
        std::ifstream infile;
        infile.open(m_output, std::ios::in | std::ios::binary);
        if (!infile.is_open()) {
            return false;
        }
        std::vector<BYTE> buffer;
        buffer.resize(filesize);
        if (buffer.size() != filesize) {
            return false;
        }
        infile.seekg(0, std::ios::beg);
        infile.read((char*)&buffer[0], filesize);
        auto readed = infile.tellg();
        if (readed != filesize) {
            return false;
        }
        infile.close();
        std::ofstream outfile;
        outfile.open(m_output, std::ios::out | std::ios::binary);
        if (!outfile.is_open()) {
            return false;
        }
        outfile.write((char*)&(_lstm_model_magic), sizeof(std::int32_t));
        outfile.write((char*)&(_lstm_model_version), sizeof(std::int32_t));
        outfile.write((char*)&(filesize), sizeof(std::int32_t));
        outfile.write((char*)buffer.data(), buffer.size());
        if (m_datasets->save(outfile)) {
            std::cout << "save success..." << std::endl;
            return true;
        }
        std::cout << "save failed..." << std::endl;
        return false;
    }

    bool CLstmModelImpl::unpackmodel(CLstmModel& model) {
        if (!m_valid) {
            return false;
        }
        if (m_modelbin.empty()) {
            return false;
        }
        if (!fs::is_regular_file(m_modelbin)) {
            return false;
        }
        std::uint32_t filesize = fs::file_size(m_modelbin);
        if (filesize == static_cast<std::uint32_t>(-1) || filesize == 0) {
            return false;
        }
        if (filesize < 12) {
            return false;
        }
        std::ifstream infile;
        infile.open(m_modelbin, std::ios::in | std::ios::binary);
        if (!infile.is_open()) {
            return false;
        }
        std::int32_t lstm_magic = 0;
        std::int32_t lstm_version = 0;
        infile.read((char*)&(lstm_magic), sizeof(std::int32_t));
        infile.read((char*)&(lstm_version), sizeof(std::int32_t));
        if (lstm_magic != _lstm_model_magic || lstm_version != _lstm_model_version) {
            infile.close();
            return loadtorchmodel(model, m_modelbin);
        }
        std::int32_t model_size = 0;
        infile.read((char*)&(model_size), sizeof(std::int32_t));
        if (model_size == 0 || filesize < 12 + model_size) {
            return false;
        }
        auto tmppath = fs::temp_directory_path().append("_model.pt");
        std::ofstream outfile(tmppath, std::ios::out | std::ios::binary);
        if (!outfile.is_open()) {
            return false;
        }
        std::vector<BYTE> buffer;
        buffer.resize(model_size);
        if (buffer.size() != model_size) {
            return false;
        }
        infile.read((char*)&buffer[0], model_size);
        auto readed = infile.tellg();
        if (readed != 12 + model_size) {
            return false;
        }
        outfile.write((char*)buffer.data(), buffer.size());
        outfile.close();
        loadtorchmodel(model, tmppath.string());
        fs::remove(tmppath);
        return m_datasets->load(infile);
    }

    bool CLstmModelImpl::loadtorchmodel(CLstmModel& model, const std::string& modelbin) {
        torch::load(model, modelbin); 
        return true;
    }

    bool CAlbertModel::train() {
        return false;
    }

    bool CAlbertModel::test() {
        return false;
    }

    bool CAlbertModel::predict(const std::string& inputpath) {
        if (!m_valid) {
            return false;
        }
        CFunduration func_duration("predicting time: ");
        m_datasets->LoadDict(CDataSets::dict_sentencepiece);
        std::cout << "Bert Model predicting start... " << std::endl;
        try {
            m_predict_result = torch::Tensor();
            m_model.to(*m_device);
            std::unique_ptr<cchips::CJsonOptions> options = std::make_unique<cchips::CJsonOptions>("CFGRES", IDR_JSONPE_CFG);
            if (!options || !options->Parse()) {
                std::cout << "load config error." << std::endl;
                return false;
            }
            auto& staticmanager = cchips::GetCStaticFileManager();
            if (!staticmanager.Initialize(std::move(options))) {
                std::cout << "Initialize failed!" << std::endl;
                return false;
            }
            std::string output;
            if (!staticmanager.Scan(inputpath, output, false) || !output.length()) {
                std::cout << "scan: " << inputpath << " failed" << std::endl;
                return false;
            }
            auto& corpusmanager = cchips::GetCTextCorpusManager().GetInstance();
            corpusmanager.Initialize(output);
            if (!corpusmanager.GeneratingModelDatasets("fasttext", output, "")) {
                std::cout << "generating fasttext corpus failed!" << std::endl;
                return false;
            }
            if (!output.length()) {
                return false;
            }

            std::atomic_uint32_t corrects = 0;
            torch::NoGradGuard no_grad;
            torch::Tensor inputids, attentionmask;
            std::tie(inputids, attentionmask) = m_datasets->LoadSample(output);
            if (inputids.defined() && attentionmask.defined()) {
                inputids = inputids.to(*m_device);
                attentionmask = attentionmask.to(*m_device);
                auto outputs = forward(inputids, attentionmask);
                m_predict_result = torch::softmax(outputs, -1);
            }
        }
        catch (const std::exception& e) {
            std::cout << "Exception occurred during model predicting: " << e.what() << std::endl;
            return false;
        }
        outputPredictResult();
        return true;
    }

    torch::Tensor CAlbertModel::forward(torch::Tensor inputids, torch::Tensor attentionmask) {
        if (!m_valid) {
            return {};
        }
        std::vector<torch::jit::IValue> inputs;
        //std::cout << inputids.sizes() << std::endl;
        //std::cout << attentionmask.sizes() << std::endl;
        inputs.push_back(inputids);
        inputs.push_back(attentionmask);
        torch::Tensor output = m_model.forward(inputs).toTensor();
        return output;
    }

    bool CAlbertModel::packmodel(const std::string& dictbin) {
        if (!m_valid) {
            return false;
        }
        std::cout << "pack bert model..." << std::endl;
        if (m_output.empty()) {
            return false;
        }
        if (!m_datasets || !m_datasets->IsValid()) {
            return false;
        }
        if (!fs::is_regular_file(m_modelbin)) {
            return false;
        }
        if (!fs::is_regular_file(dictbin)) {
            return false;
        }
        std::uint32_t modelsize = fs::file_size(m_modelbin);
        if (modelsize == static_cast<std::uint32_t>(-1) || modelsize == 0) {
            return false;
        }
        std::ifstream modelfile;
        modelfile.open(m_modelbin, std::ios::in | std::ios::binary);
        if (!modelfile.is_open()) {
            return false;
        }
        std::vector<BYTE> modelbuffer;
        modelbuffer.resize(modelsize);
        if (modelbuffer.size() != modelsize) {
            return false;
        }
        modelfile.seekg(0, std::ios::beg);
        modelfile.read((char*)&modelbuffer[0], modelsize);
        auto readed = modelfile.tellg();
        if (readed != modelsize) {
            return false;
        }
        modelfile.close();
        std::uint32_t dictsize = fs::file_size(dictbin);
        if (dictsize == static_cast<std::uint32_t>(-1) || dictsize == 0) {
            return false;
        }
        std::ifstream dictfile;
        dictfile.open(dictbin, std::ios::in | std::ios::binary);
        if (!dictfile.is_open()) {
            return false;
        }
        std::vector<BYTE> dictbuffer;
        dictbuffer.resize(dictsize);
        if (dictbuffer.size() != dictsize) {
            return false;
        }
        dictfile.seekg(0, std::ios::beg);
        dictfile.read((char*)&dictbuffer[0], dictsize);
        readed = dictfile.tellg();
        if (readed != dictsize) {
            return false;
        }
        dictfile.close();
        std::ofstream outfile;
        outfile.open(m_output, std::ios::out | std::ios::binary);
        if (!outfile.is_open()) {
            return false;
        }
        outfile.write((char*)&(_albert_model_magic), sizeof(std::int32_t));
        outfile.write((char*)&(_albert_model_version), sizeof(std::int32_t));
        outfile.write((char*)&(modelsize), sizeof(std::int32_t));
        outfile.write((char*)&(dictsize), sizeof(std::int32_t));
        outfile.write((char*)modelbuffer.data(), modelbuffer.size());
        outfile.write((char*)dictbuffer.data(), dictbuffer.size());
        std::cout << "save success..." << std::endl;
        return true;
    }

    bool CAlbertModel::unpackmodel() {
        if (!m_valid) {
            return false;
        }
        if (m_modelbin.empty()) {
            return false;
        }
        if (!fs::is_regular_file(m_modelbin)) {
            return false;
        }
        std::uint32_t filesize = fs::file_size(m_modelbin);
        if (filesize == static_cast<std::uint32_t>(-1) || filesize == 0) {
            return false;
        }
        if (filesize < 16) {
            return false;
        }
        std::ifstream infile;
        infile.open(m_modelbin, std::ios::in | std::ios::binary);
        if (!infile.is_open()) {
            return false;
        }
        std::int32_t bert_magic = 0;
        std::int32_t bert_version = 0;
        infile.read((char*)&(bert_magic), sizeof(std::int32_t));
        infile.read((char*)&(bert_version), sizeof(std::int32_t));
        if (bert_magic != _albert_model_magic || bert_version != _albert_model_version) {
            infile.close();
            try {
                m_model = torch::jit::load(m_modelbin);
                m_model.eval();
            }
            catch (const c10::Error& e) {
                std::cout << "torch jit load model crash: " << e.what() << std::endl;
                m_valid = false;
                return m_valid;
            }
            return true;
        }
        std::int32_t model_size = 0;
        infile.read((char*)&(model_size), sizeof(std::int32_t));
        if (model_size == 0 || filesize < 12 + model_size) {
            return false;
        }
        std::int32_t dict_size = 0;
        infile.read((char*)&(dict_size), sizeof(std::int32_t));
        if (dict_size == 0 || filesize < 16 + model_size + dict_size) {
            return false;
        }
        auto tmppath = fs::temp_directory_path().append("_model.pt");
        std::ofstream outfile(tmppath, std::ios::out | std::ios::binary);
        if (!outfile.is_open()) {
            return false;
        }
        std::vector<BYTE> buffer;
        buffer.resize(model_size);
        if (buffer.size() != model_size) {
            return false;
        }
        infile.read((char*)&buffer[0], model_size);
        auto readed = infile.tellg();
        if (readed != 16 + model_size) {
            return false;
        }
        outfile.write((char*)buffer.data(), buffer.size());
        outfile.close();
        try {
            m_model = torch::jit::load(tmppath.string());
            m_model.eval();
        }
        catch (const c10::Error& e) {
            std::cout << "torch jit load model crash: " << e.what() << std::endl;
            m_valid = false;
            fs::remove(tmppath);
            return m_valid;
        }
        fs::remove(tmppath);
        return m_datasets->loadsentencemodel(infile, dict_size);
    }
} // namespace cchips
