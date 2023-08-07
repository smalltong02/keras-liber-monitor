
import torch
import time
import os
import re
from tqdm import tqdm
import sentencepiece as spm
import torch.nn as nn
import torch.optim as optim
from transformers import AlbertTokenizer, AlbertForSequenceClassification #AlbertModel #AlbertForSequenceClassification
from torch.utils.data import DataLoader, Dataset

label_map = {
    "__label_forticlient__": 0,
    "__label_cert_manage_tool__": 1,
    "__label_putty__": 2,
    "__label_unknown__": 3,
    "__label_teams__": 4
}

# Define a custom dataset for training and testing
class CustomDataset(Dataset):
    def __init__(self, file_path, tokenizer, sentences_length=8000):
        self.tokenizer = tokenizer
        self.sentences_length = sentences_length
        self.labels, self.texts = self.read_data(file_path)

    def __len__(self):
        return len(self.labels)

    def __getitem__(self, idx):
        label = self.labels[idx]
        long_text = self.texts[idx]
        #if len(long_text) > self.sentences_length:
        #    print("long_text: ", len(long_text))
        long_text = long_text[0:self.sentences_length]
        max_length = 512
        stride = 256
        split_texts = self.split_text_by_word_limit(long_text, max_length)

        label_list = []
        input_ids_list = []
        attention_mask_list = []
        for input_text in split_texts:
            inputs = self.tokenizer.encode_plus(input_text, add_special_tokens=True, return_tensors="pt", max_length=512, padding=False, truncation=True)
            decoded_text = tokenizer.decode(inputs['input_ids'][0])
            #print(len(decoded_text))
            input_ids_list.append(inputs["input_ids"])
            attention_mask_list.append(inputs["attention_mask"])
            label_list.append(label)

        input_ids = torch.cat(input_ids_list, dim=0)
        attention_mask = torch.cat(attention_mask_list, dim=0)
        labels = torch.tensor(label_list)
        return input_ids, attention_mask, labels

    def split_text_by_word_limit(self, text, word_limit=512):
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
                break
                #splitted_texts.append(current_text)
                #current_text = word
                #word_length = 0

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
                int_label = label_map[label]
                labels.append(int_label)
                texts.append(text)
        return labels, texts

class LinearClassifier(nn.Module):
    def __init__(self, input_dim, output_dim):
        super(LinearClassifier, self).__init__()
        self.fc = nn.Linear(input_dim, output_dim)

    def forward(self, x):
        return self.fc(x)

# Tokenizer and model
start_time = time.time()
device = torch.device('cuda' if torch.cuda.is_available() else 'cpu')
print(device)
tokenizer = AlbertTokenizer.from_pretrained('albert-base-v2')
if os.path.exists('H:\\machine_learning\\albert\\albert_model.pt'):
    model = torch.load('H:\\machine_learning\\albert\\albert_model.pt', map_location=device)
    print("success load model from H:\\machine_learning\\albert\\albert_model.pt")
else:
    model = AlbertForSequenceClassification.from_pretrained('albert-base-v2', num_labels=5)
    print("success create new albert-base-v2 model")
um_trainable_params = sum(p.numel() for p in model.parameters() if p.requires_grad)
print(f"Total trainable parameters：{um_trainable_params}")

# Dataset and DataLoader
traindata_path = "H:\\machine_learning\\fasttext_train\\train.txt"
trainset = CustomDataset(traindata_path, tokenizer)
batch_size = 1
train_loader = DataLoader(trainset, batch_size=batch_size, shuffle=True)

# Training loop (Replace this with actual training code)
optimizer = torch.optim.AdamW(model.parameters(), lr=1e-05)
model.to(device)
chunks_size = 1
num_classes = 5
window_size = 512
num_epochs = 100
model.train()

for epoch in range(num_epochs):
    average_loss = 0.0
    total_loss = 0.0
    num_batches = 0  
    for batch in tqdm(trainset, total=len(trainset)):
        input_ids, attention_mask, labels = batch
        input_ids = input_ids.to(device)
        attention_mask = attention_mask.to(device)
        labels = labels.to(device)
        #print(input_ids.shape)
        #print(attention_mask.shape)
        #print(labels.shape)

        windowed_output = []
        windowed_labels = []
        num_chunks = input_ids.size(0) // chunks_size
        chunks_ids = torch.chunk(input_ids, num_chunks, dim=0)
        chunks_mask = torch.chunk(attention_mask, num_chunks, dim=0)
        chunks_label = torch.chunk(labels, num_chunks, dim=0)
        for chunk1, chunk2, chunk3 in zip(chunks_ids, chunks_mask, chunks_label):
            #with torch.no_grad():
                outputs = model.forward(input_ids=chunk1, attention_mask=chunk2)
                #print(outputs.logits)
                #print(outputs.logits.shape)
                #last_hidden_state = outputs.last_hidden_state
                #print(last_hidden_state.shape)
                #num_batchs = last_hidden_state.size(0)
                #for i in range(num_batchs):
                    #windowed_output.append(last_hidden_state[i])
                #expanded_label = chunk3.repeat(512)
                #windowed_labels.append(expanded_label)
                windowed_output.append(outputs.logits)
                windowed_labels.append(chunk3)

        combined_last_hidden_state = torch.cat(windowed_output, dim=0)
        #print(combined_last_hidden_state.shape)
        criterion = nn.CrossEntropyLoss()
        labels_tensor = torch.cat(windowed_labels, dim=0)
        loss = criterion(combined_last_hidden_state, labels_tensor)
        optimizer = torch.optim.AdamW(model.parameters(), lr=1e-05)
        optimizer.zero_grad()
        loss.backward()
        #print("loss:", loss.item())
        optimizer.step()
        total_loss += loss.item()
        num_batches = num_batches + 1

        #print(len(windowed_output))
        #print(len(windowed_labels))
        #combined_last_hidden_state = torch.cat(windowed_output, dim=0)
        #print(combined_last_hidden_state.shape)
        #classifier = nn.Linear(combined_last_hidden_state.shape[-1], num_classes)
        #classifier = classifier.to(device)
        #logits = classifier(combined_last_hidden_state)
        #criterion = nn.CrossEntropyLoss()
        #labels_tensor = torch.cat(windowed_labels, dim=0)
        #print(logits)
        #print(labels_tensor)
        #loss = criterion(logits.view(-1, num_classes), labels_tensor)
        #optimizer.zero_grad()
        #loss.backward()
        #print("loss:", loss.item())
        #optimizer.step()
        #total_loss += loss.item()
        #num_batches = num_batches + 1

    average_loss = total_loss / num_batches
    print("Epoch: ", epoch + 1, ", average loss: ", average_loss)

end_time = time.time()
run_time = end_time - start_time
print("running: ", run_time, " seconds")
torch.save(model, 'H:\\machine_learning\\albert\\albert_model.pt')

"""
# Inference (Replace this with actual inference code)
testdata_path = "H:\\machine_learning\\fasttext_test\\test.txt"
testset = CustomDataset(testdata_path, tokenizer)
test_loader = DataLoader(testset, batch_size=2, shuffle=False)

model.eval()
with torch.no_grad():
    for batch in test_loader:
        input_ids = batch['input_ids'].to(device)
        attention_mask = batch['attention_mask'].to(device)

        outputs = model(input_ids=input_ids, attention_mask=attention_mask)
        logits = outputs.logits
        predictions = torch.argmax(logits, dim=1)

        print("Predictions:", predictions)
"""
