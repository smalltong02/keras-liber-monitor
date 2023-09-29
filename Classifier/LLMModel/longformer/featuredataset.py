import re
import torch
from torch.utils.data import Dataset
from collections import OrderedDict

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

        return self.datasets[idx]["context"], index

    def read_data(self, file_path):
        datasets = []
        label_pattern = r'(__label_[\w-]+)'
        with open(file_path, "r", encoding="utf-8") as file:
            for line in file:
                line = line.strip()
                line_labels = re.findall(label_pattern, line)
                line_text = re.sub(label_pattern, '', line)
                line_text = line_text.strip()
                data = {'labels': line_labels, 'context': line_text}
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