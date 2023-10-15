import re
import torch
from torch.utils.data import Dataset
from collections import OrderedDict
from datasets import *

class HuggClassificationDataset():
    def __init__(self, datapath, tokenizer, labeldeep):
        self.labeldeep = labeldeep
        self.tokenizer = tokenizer
        self.orgdata = load_dataset("text", data_files=datapath, split="train")
        self.labels = self.read_labels()
        self.orgdata = self.orgdata.train_test_split()#test_size=0.1)
        self.tokenized_datasets = self.orgdata.map(self.preprocess_datasets, batched=True, remove_columns=self.orgdata["train"].column_names)

    def get_tokenized_datasets(self):
        return self.tokenized_datasets

    def preprocess_datasets(self, example):
        if self.labeldeep != 1:
            return
        labels = []
        texts = []
        label_pattern = r'(__label_[\w-]+)'
        sep_pattern = r'#SEP#'
        for line in example["text"]:
            line = line.strip()
            line_labels = re.findall(label_pattern, line)
            line_text = re.sub(label_pattern, '', line)
            line_text = line_text.strip()
            line_text = re.sub(sep_pattern, self.tokenizer.sep_token, line_text)
            labels.append(self.labels.index(line_labels[0]))
            texts.append(line_text)
        datadict = {'labels': labels, 'content': texts}
        model_inputs = self.tokenizer(datadict["content"], max_length=512, truncation=True)
        model_inputs["labels"] = datadict["labels"]
        return model_inputs

    def read_labels(self):
        labels = OrderedDict()
        label_pattern = r'(__label_[\w-]+)'
        for line in self.orgdata["text"]:
            line = line.strip()
            line_labels = re.findall(label_pattern, line)
            for i in range(self.labeldeep):
                if i < len(line_labels):
                    labels[line_labels[i]] = None
        return list(labels.keys())

        

class AppClassificationDataset(Dataset):
    def __init__(self, datapath, tokenizer, labeldeep):
        self.labeldeep = labeldeep
        self.tokenizer = tokenizer
        self.datasets = self.read_data(datapath)
        self.labels = self.read_labels()

    def __len__(self):
        return len(self.datasets)

    def __getitem__(self, idx):
        labels = []
        for i in range(self.labeldeep):
            label = self.datasets[idx]["labels"][i]
            try:
                index = self.labels.index(label)
                labels.append(index)
            except ValueError:
                return "", -1

        return self.datasets[idx]["content"], index

    def read_data(self, file_path):
        datasets = []
        label_pattern = r'(__label_[\w-]+)'
        sep_pattern = r'#SEP#'
        with open(file_path, "r", encoding="utf-8") as file:
            for line in file:
                line = line.strip()
                line_labels = re.findall(label_pattern, line)
                line_text = re.sub(label_pattern, '', line)
                line_text = line_text.strip()
                line_text = re.sub(sep_pattern, self.tokenizer.sep_token, line_text)
                data = {'labels': line_labels, 'content': line_text}
                datasets.append(data)
        return datasets

    def read_labels(self):
        labels = OrderedDict()
        for data in self.datasets:
            data_labels = data['labels']
            for i in range(self.labeldeep):
                if i < len(data_labels):
                    labels[data_labels[i]] = None
        return list(labels.keys())