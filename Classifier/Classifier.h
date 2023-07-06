#pragma once
#include <opencv2/opencv.hpp>
#include <torch/torch.h>

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
