#include "Classifier.h"

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
