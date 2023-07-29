#include "resource.h"
#include "Classifier.h"

namespace cchips {

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
        if (!fs::is_regular_file(dataset_path)) {
            return;
        }
        if (!LoadDataset(dataset_path)) {
            return;
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
        }
        return false;
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
        while(ss >> word) {
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
        for (; word_counts < m_sequences_size; word_counts++) {
            sequences.push_back(m_dict[SPECIAL_CORPUS_PAD]);
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
    std::tuple<torch::Tensor, torch::Tensor> CDataSets::LoadBatch(const std::vector<torch::data::Example<>>& batch) const {
        if (!batch.size()) {
            return {};
        }
        std::vector<torch::Tensor> data_batch;
        std::vector<torch::Tensor> target_batch;

        for (const auto& bat : batch) {
            data_batch.push_back(bat.data);
            target_batch.push_back(bat.target);
        }

        torch::Tensor data = torch::stack(data_batch);
        torch::Tensor target = torch::cat(target_batch);
        data = torch::transpose(data, 0, 1);
//        std::cout << data.sizes() << std::endl;
//        std::cout << target.sizes() << std::endl;
        return {data, target};
    }

    bool CFastTextModel::train() {
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

        //CDataSets datasets(m_ratio);
        //if (!datasets.splitDatasets(m_input)) {
        //    return false;
        //}
        //std::vector<fs::path> trainsets;
        //std::vector<fs::path> testsets;
        //if (!datasets.getTrainSets(trainsets)) {
        //    return false;
        //}
        //if (!datasets.getTestSets(testsets)) {
        //    return false;
        //}
        //if (!trainsets.size() || !testsets.size()) {
        //    return false;
        //}
        fasttext::Args args = fasttext::Args();
        
        args.input = m_input;
        switch(m_subtype) {
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
            args.epoch = 100;
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
        return false;
    }

    bool CFastTextModel::predict() {
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
            m_ftmodel->predictLine(in, predictions, m_k, threshold);
            m_k = 1;
            if (predictions.size()) {
                for (auto pre : predictions) {
                    std::cout << "predict class: " << pre.second << " similarity: " << pre.first << std::endl;
                }
            }
            else {
                std::cout << "predict failed!" << std::endl;
            }
            return true;
        }
        catch (const std::exception& e) {
            std::cout << "Exception occurred during Fasttext model predicting: " << e.what() << std::endl;
            return false;
        }
    }

    torch::Tensor CGruModel::forward(torch::Tensor input)
    {
        if (!m_valid) {
            return {};
        }
        torch::Tensor gru_output, gru_h_n;
        std::tie(gru_output, gru_h_n) = m_grumodel->forward(input);
        torch::Tensor last_output = gru_output.select(0, gru_output.size(0) - 1);
        torch::Tensor output = m_linearmodel->forward(last_output);
        return output;
    }

    bool CGruModel::train() {
        if (!m_valid) {
            return false;
        }
        std::cout << "GRU Model training start... " << std::endl;
        torch::nn::CrossEntropyLoss criterion;
        torch::optim::SGD optimizer(parameters(), torch::optim::SGDOptions(0.01));
        try {
            for (int epoch = 0; epoch < m_num_epochs; ++epoch) {
                auto data_loader = torch::data::make_data_loader(
                    *m_datasets,
                    torch::data::DataLoaderOptions().batch_size(m_datasets->getBatchsize()).workers(10));
                std::uint32_t num_samples = 0;
                std::uint32_t num_batches = 0;
                std::float_t average_loss = 0.0;
                std::float_t total_loss = 0.0;
                for (const auto& batch : *data_loader)
                {
                    torch::Tensor inputs, targets;
                    std::tie(inputs, targets) = m_datasets->LoadBatch(batch);
                    if (inputs.defined() && targets.defined()) {
                        if (m_bcpu) {
                            inputs = inputs.cpu();
                            targets = targets.cpu();
                        }
                        else {
                            inputs = inputs.cuda();
                            targets = targets.cuda();
                        }
                        //std::cout << inputs.sizes() << std::endl;
                        //std::cout << targets.sizes() << std::endl;

                        auto outputs = forward(inputs);
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
            std::cout << "Exception occurred during GRU model training: " << e.what() << std::endl;
        }
        std::cout << "GRU Model training end... " << std::endl;
        if (m_output.length()) {
            std::cout << "save GRU Model..." << std::endl;
            torch::serialize::OutputArchive output_archive;
            save(output_archive);
            output_archive.save_to(m_output);
        }
        return true; 
    }

    bool CGruModel::test() {
        if (!m_valid) {
            return false;
        }
        try {

        }
        catch (const std::exception& e) {
            std::cout << "Exception occurred during GRU model predicting: " << e.what() << std::endl;
            return false;
        }
        return true;
    }
    
    bool CGruModel::predict() {
        if (!m_valid) {
            return false;
        }
        try {

        }
        catch (const std::exception& e) {
            std::cout << "Exception occurred during GRU model predicting: " << e.what() << std::endl;
            return false;
        }
        return true;
    }
} // namespace cchips
