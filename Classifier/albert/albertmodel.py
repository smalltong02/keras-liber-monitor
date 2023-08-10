import torch
import time
import os
from tqdm import tqdm
import sentencepiece as spm
import torch.nn as nn
import torch.optim as optim
from transformers import AlbertTokenizer, AlbertForSequenceClassification
from cusdataset import CustomDataset

class LinearClassifier(nn.Module):
    def __init__(self, inputdim, outputdim):
        super(LinearClassifier, self).__init__()
        self.fc = nn.Linear(inputdim, outputdim)

    def forward(self, x):
        return self.fc(x)

class InnerAlbertModel(nn.Module):
    def __init__(self, albert_model):
        super(InnerAlbertModel, self).__init__()
        self.model = albert_model

    def forward(self, input_ids, attention_mask):
        output = self.model(input_ids=input_ids, attention_mask=attention_mask)
        return output.logits

class InnerAlbertInstance:
    def __init__(self, name, input, output, modelbin):
        self.modelname = name
        self.label_map = {
            "__label_forticlient__": 0,
            "__label_cert_manage_tool__": 1,
            "__label_putty__": 2,
            "__label_unknown__": 3,
            "__label_teams__": 4
        }
        if output is None:
            output = ""
        if modelbin is None:
            modelbin = ""
        self.input = input
        self.output = output
        self.modelbin = modelbin
        self.device = torch.device('cuda' if torch.cuda.is_available() else 'cpu')
        self.tokenizer = AlbertTokenizer.from_pretrained('albert-base-v2')
        if os.path.exists(self.modelbin):
            self.model = torch.load(self.modelbin, map_location=self.device)
            print("success load model from ", self.modelbin)
        else:
            print(len(self.label_map))
            self.model = AlbertForSequenceClassification.from_pretrained('albert-base-v2', num_labels=len(self.label_map))
        self.model.to(self.device)
        print(self.device)
        um_trainable_params = sum(p.numel() for p in self.model.parameters() if p.requires_grad)
        print(f"Total trainable parameters：{um_trainable_params}")

    def getlabel(self, intlabel):
        for label, val in self.label_map.items():
            if val == intlabel:
                return label
        return None

    def train(self):
        print(f"Training {self.modelname} model start!")
        start_time = time.time()
        if self.input == "" or self.output == "":
            return
        trainset = CustomDataset(self.input, self.tokenizer, self.label_map)
        optimizer = torch.optim.AdamW(self.model.parameters(), lr=1e-05)
        num_epochs = 1
        intermodel = InnerAlbertModel(self.model)
        intermodel.train()
        for epoch in range(num_epochs):
            average_loss = 0.0
            total_loss = 0.0
            num_batches = 0
            for batch in tqdm(trainset, total=len(trainset)):
                input_ids, attention_mask, labels = batch
                input_ids = input_ids.to(self.device)
                attention_mask = attention_mask.to(self.device)
                labels = labels.to(self.device)
                #print(input_ids.shape)
                #print(attention_mask.shape)
                outputs = intermodel.forward(input_ids, attention_mask)
                criterion = nn.CrossEntropyLoss()
                loss = criterion(outputs, labels)
                optimizer = torch.optim.AdamW(intermodel.model.parameters(), lr=1e-05)
                optimizer.zero_grad()
                loss.backward()
                optimizer.step()
                total_loss += loss.item()
                num_batches = num_batches + 1
            average_loss = total_loss / num_batches
            print("Epoch: ", epoch + 1, ", average loss: ", average_loss)

        output_path = self.output + '\\albert_model.pt'
        script_model = torch.jit.trace(intermodel, (input_ids, attention_mask), strict=True)
        torch.jit.save(script_model, output_path)
        #script_model = torch.jit.trace(self.model, (input_ids, attention_mask), strict=True)
        #torch.jit.save(script_model, output_path)
        end_time = time.time()
        run_time = end_time - start_time
        print(f"running: ", run_time, "seconds")

    def test(self):
        print(f"Testing {self.modelname} model start!")
        start_time = time.time()
        if self.input == "" or self.modelbin == "":
            return
        accuracy = 0.0
        corrects = 0
        testset = CustomDataset(self.input, self.tokenizer, self.label_map)
        self.model.eval()
        with torch.no_grad():
            for batch in tqdm(testset, total=len(testset)):
                input_ids, attention_mask, labels = batch
                input_ids = input_ids.to(self.device)
                attention_mask = attention_mask.to(self.device)
                outputs = self.model(input_ids=input_ids, attention_mask=attention_mask)
                logits = outputs.logits
                predictions = torch.argmax(logits, dim=1)
                t1 = predictions[0].item()
                t2 = labels[0].item()
                if t1 == t2:
                    corrects = corrects + 1

        accuracy = corrects / len(testset)
        formatted_accuracy = "{:.2%}".format(accuracy)
        print(f"accuracy:", formatted_accuracy)

    def predict(self):
        print(f"Predicting {self.modelname} model start!")
        start_time = time.time()
        if self.input == "" or self.modelbin == "":
            return
        predictset = CustomDataset(self.input, self.tokenizer, self.label_map)
        self.model.eval()
        with torch.no_grad():
            for batch in predictset:
                input_ids, attention_mask, _ = batch
                input_ids = input_ids.to(self.device)
                attention_mask = attention_mask.to(self.device)
                outputs = self.model(input_ids=input_ids, attention_mask=attention_mask)
                logits = outputs.logits
                predictions = torch.argmax(logits, dim=1)
                t1 = predictions[0].item()
                #predict_result = torch.softmax(logits, dim=-1)
                

        print(f"predict label: ", self.getlabel(t1))