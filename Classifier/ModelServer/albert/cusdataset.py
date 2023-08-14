import re
import torch
from torch.utils.data import Dataset

class CustomDataset(Dataset):
    def __init__(self, filepath, tokenizer, labelmap, sentences_length=8000):
        self.tokenizer = tokenizer
        self.sentences_length = sentences_length
        self.labelmap = labelmap
        self.maxlength = 512
        self.labels, self.texts = self.read_data(filepath)

    def __len__(self):
        return len(self.labels)

    def __getitem__(self, idx):
        label = self.labels[idx]
        long_text = self.texts[idx]
        long_text = long_text[0:self.sentences_length]
        split_texts = self.split_text_by_word_limit(long_text, self.maxlength)

        label_list = []
        input_ids_list = []
        attention_mask_list = []
        for input_text in split_texts:
            inputs = self.tokenizer.encode_plus(input_text, add_special_tokens=True, return_tensors="pt", max_length=512, padding=False, truncation=True)
            #print(inputs)
            input_ids_list.append(inputs["input_ids"])
            attention_mask_list.append(inputs["attention_mask"])
            label_list.append(label)

        input_ids = torch.cat(input_ids_list, dim=0)
        attention_mask = torch.cat(attention_mask_list, dim=0)
        labels = torch.tensor(label_list)
        return input_ids, attention_mask, labels

    def split_text_by_word_limit(self, text, word_limit=512, nums = 1):
        words = text.split()
        current_text = ""
        splitted_texts = []
        word_length = 0
        for word in words:
            word_length = word_length + 1
            if word_length <= word_limit:
                if current_text:
                    current_text += " "
                current_text += word
            else:
                nums = nums - 1
                if nums == 0:
                    break
                splitted_texts.append(current_text)
                current_text = word
                word_length = 1

        if current_text:
            splitted_texts.append(current_text)
        return splitted_texts

    def read_data(self, file_path):
        labels = []
        texts = []
        i = 0
        with open(file_path, "r", encoding="utf-8") as file:
            for line in file:
                label, text = line.strip().split(" ", 1)
                text = re.sub(r'#(.*?)# ', '', text)
                int_label = self.labelmap[label]
                labels.append(int_label)
                texts.append(text)
        return labels, texts
