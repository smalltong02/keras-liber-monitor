import torch
import time
import os
from tqdm import tqdm
import torch.nn as nn
import torch.optim as optim
from transformers import LongformerTokenizerFast, LongformerForSequenceClassification

class InnerLongformerModel(nn.Module):
    def __init__(self, model):
        super(InnerLongformerModel, self).__init__()
        self.model = model

    def forward(self, input_ids, attention_mask):
        output = self.model(input_ids=input_ids, attention_mask=attention_mask)
        return output.logits

class InnerLongformerClassificationInstance:
    def __init__(self, name, input, output, config, modelbin):
        self.modelname = name
        if output is None:
            output = ""
        if modelbin is None:
            modelbin = ""
        self.input = input
        self.output = output
        self.config = config
        self.modelbin = modelbin
        self.device = torch.device('cuda' if torch.cuda.is_available() else 'cpu')
        if self.modelbin is None or self.modelbin == "":
            self.modelbin = 'allenai/longformer-base-4096'
        
        self.tokenizer = LongformerTokenizerFast.from_pretrained(self.modelbin)
        if os.path.isfile(self.modelbin):
            self.model = torch.load(self.modelbin, map_location=self.device)
            print("success load model from ", self.modelbin)
        else:
            self.model = LongformerForSequenceClassification.from_pretrained(self.modelbin)
            print("success load model from ", self.modelbin)

        self.model.to(self.device)
        print(self.device)
        um_trainable_params = sum(p.numel() for p in self.model.parameters() if p.requires_grad)
        print(f"Total trainable parameters：{um_trainable_params}")

    def train(self):
        print(self.tokenizer.vocab_size)
        inputs = self.tokenizer("I have a dreaming!", return_tensors="pt")

        print(self.model.config)
        output = self.model(**inputs)
        print(output)

    def test(self):
        pass

    def predict(self):
        pass