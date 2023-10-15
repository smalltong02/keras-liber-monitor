import torch
import time
import os
from tqdm import tqdm
import torch.nn as nn
import torch.optim as optim
import evaluate
from transformers import LongformerTokenizerFast, LongformerForSequenceClassification, DataCollatorWithPadding, Trainer, TrainingArguments
from longformer.featuredataset import AppClassificationDataset, HuggClassificationDataset
from torch.utils.data import random_split, DataLoader

class InnerLongformerModel(nn.Module):
    def __init__(self, model):
        super(InnerLongformerModel, self).__init__()
        self.model = model

    def forward(self, input_ids, attention_mask):
        output = self.model(input_ids=input_ids, attention_mask=attention_mask)
        return output.logits

class ConfiguredMetric:
    def __init__(self, metric, *metric_args, **metric_kwargs):
        self.metric = metric
        self.metric_args = metric_args
        self.metric_kwargs = metric_kwargs
    
    def add(self, *args, **kwargs):
        return self.metric.add(*args, **kwargs)
    
    def add_batch(self, *args, **kwargs):
        return self.metric.add_batch(*args, **kwargs)

    def compute(self, *args, **kwargs):
        return self.metric.compute(*args, *self.metric_args, **kwargs, **self.metric_kwargs)

    @property
    def name(self):
        return self.metric.name

    def _feature_names(self):
        return self.metric._feature_names()

class ConfiguredFineTunning:
    def __init__(self, model, bitfit=True):
        if bitfit:
            num_params = 0
            for name, param in model.named_parameters():
                if "bias" not in name:
                    param.required_grad = False
                else:
                    num_params += param.numel()
            percentage = num_params / sum(param.numel() for param in model.parameters())
            print(f" Bitfat=True Bias params: {num_params}, percentage: {percentage}")

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
        self.lr = 0.011
        self.modelbin = modelbin
        self.device = torch.device('cuda' if torch.cuda.is_available() else 'cpu')
        if self.modelbin is None or self.modelbin == "":
            self.modelbin = 'allenai/longformer-base-4096'
        self.tokenizer = LongformerTokenizerFast.from_pretrained(self.modelbin)
        if os.path.isfile(self.modelbin):
            self.model = torch.load(self.modelbin, map_location=self.device)
            print("success load model from ", self.modelbin)
        else:
            num_labels = 5
            self.model = LongformerForSequenceClassification.from_pretrained(self.modelbin, num_labels=num_labels)
            print("success load model from ", self.modelbin)
        self.clf_metrics = evaluate.combine([
            evaluate.load("accuracy"),
            ConfiguredMetric(evaluate.load("f1") , average="macro"),
            ConfiguredMetric(evaluate.load("recall"), average="macro"),
            ConfiguredMetric(evaluate.load("precision"), average="macro"),
            ])
        #ConfiguredFineTunning(self.model)
        self.model.to(self.device)
        print(self.device)
        um_trainable_params = sum(p.numel() for p in self.model.parameters() if p.requires_grad)
        print(f"Total trainable parameters：{um_trainable_params}")

    def eval_metric(self, eval_predict):
        acc_metric = evaluate.load("accuracy")
        f1_metric = ConfiguredMetric(evaluate.load("f1") , average="macro")
        recall_metric = ConfiguredMetric(evaluate.load("recall"), average="macro")
        precision_metric = ConfiguredMetric(evaluate.load("precision"), average="macro")
        predictions, labels = eval_predict
        predictions = predictions.argmax(axis=-1)
        acc = acc_metric.compute(predictions=predictions, references=labels)
        f1 = f1_metric.compute(predictions=predictions, references=labels)
        recall = recall_metric.compute(predictions=predictions, references=labels)
        precision = precision_metric.compute(predictions=predictions, references=labels)
        acc.update(f1)
        acc.update(recall)
        acc.update(precision)
        return acc

    def evalute(self, validloader):
        predict_label = []
        self.model.eval()
        with torch.inference_mode():
            for batch in tqdm(validloader, desc='Validating:'):
                if torch.cuda.is_available():
                    batch = {k: v.cuda() for k,v in batch.items()}
                output = self.model(**batch)
                pred = torch.argmax(output.logits, dim=-1)
                self.clf_metrics.add_batch(predictions=pred.long(), references=batch["labels"].long())
                predict_label.append((batch["labels"].long().item(), pred.long().item()))
        print('predict_label: ', predict_label)
        return self.clf_metrics.compute()

    # def train(self):
    #     print(f"Training {self.modelname} model start!")
    #     start_time = time.time()
    #     if self.input == "" or self.output == "":
    #         return
    #     hugg_datamodel = HuggClassificationDataset(self.input, self.tokenizer, 1)
    #     datasets = hugg_datamodel.get_tokenized_datasets()
    #     collator = DataCollatorWithPadding(tokenizer=self.tokenizer)
    #     trainsets, validsets = datasets["train"], datasets["test"]

    #     train_args = TrainingArguments(output_dir="./longformer/checkpoints", 
    #                                    per_device_train_batch_size=1, 
    #                                    per_device_eval_batch_size=1,
    #                                    logging_steps=100,
    #                                    evaluation_strategy="epoch",
    #                                    save_strategy="epoch",
    #                                    save_total_limit=5,
    #                                    learning_rate=2e-5,
    #                                    weight_decay=0.01,
    #                                    load_best_model_at_end=True,
    #                                    num_train_epochs=5)

    #     trainer = Trainer(model=self.model,
    #                       args=train_args, 
    #                       train_dataset=trainsets, 
    #                       eval_dataset=validsets, 
    #                       data_collator=collator,
    #                       compute_metrics=self.eval_metric)
        
    #     trainer.train()
    #     trainer.evaluate()

        # trainloader = DataLoader(trainsets, batch_size=4, collate_fn=collator, shuffle=True)
        # validloader = DataLoader(validsets, batch_size=4, collate_fn=collator, shuffle=True)
        # print(f'trainsets: {len(trainsets)}')
        # print(f'validsets: {len(validsets)}')
        # optimizer = optim.AdamW(self.model.parameters(), lr=2e-5)
        # epochs = 3
        # log_steps = 100
        # cur_step = 0
        # for ep in range(epochs):
        #     self.model.train()
        #     for batch in tqdm(trainloader, desc='Training'):
        #         if torch.cuda.is_available():
        #             batch = {k: v.cuda() for k,v in batch.items()}
        #         optimizer.zero_grad()
        #         output = self.model(**batch)

        #         #pred = torch.argmax(output.logits, dim=-1)
        #         #self.clf_metrics.add_batch(predictions=pred.long(), references=batch["labels"].long())
        #         #elf = self.clf_metrics.compute()
        #         #print(f"{elf}")

        #         output.loss.backward()
        #         optimizer.step()
        #         #if cur_step != 0 and cur_step % log_steps == 0:
        #         #    print(f"epoch: {ep}, cur_step: {cur_step}, loss: {output.loss.item()}")
        #         #cur_step += 1
        #     acc = self.evalute(validloader)
        #     print(f"epoch: {ep}, {acc}")
        #     end_time = time.time()
        #     run_time = end_time - start_time
        #     print(f"running: ", run_time, "seconds")
        # end_time = time.time()
        # run_time = end_time - start_time
        # print(f"running: ", run_time, "seconds")

    def lr_lambda(self):
        self.lr = 0.9 * self.lr
        print("current learning rate: ", self.lr)
        return self.lr

    def train(self):
        print(f"Training {self.modelname} model start!")
        start_time = time.time()
        if self.input == "" or self.output == "":
            return
        datasets = AppClassificationDataset(self.input, self.tokenizer, 1)
        trainsets1, trainsets2, trainsets3, trainsets4, validsets = random_split(datasets, lengths=[0.01, 0.39, 0.2, 0.2, 0.2])
        trainloader1 = DataLoader(trainsets1, batch_size=1, shuffle=True, collate_fn=self.collate_func)
        trainloader2 = DataLoader(trainsets2, batch_size=1, shuffle=True, collate_fn=self.collate_func)
        trainloader3 = DataLoader(trainsets3, batch_size=1, shuffle=True, collate_fn=self.collate_func)
        trainloader4 = DataLoader(trainsets4, batch_size=1, shuffle=True, collate_fn=self.collate_func)
        validloader = DataLoader(validsets, batch_size=1, shuffle=True, collate_fn=self.collate_func)
        print(f'trainsets1: {len(trainsets1)}')
        print(f'trainsets2: {len(trainsets2)}')
        print(f'trainsets3: {len(trainsets3)}')
        print(f'trainsets4: {len(trainsets4)}')
        print(f'validsets: {len(validsets)}')
        epochs = 100
        num_loaders = 1
        log_steps = 100
        cur_step = 0
        for ep in range(epochs):
            optimizer = optim.AdamW(self.model.parameters(), lr=self.lr_lambda())
            for lo in range(num_loaders):
                self.model.train()
                if lo == 0:
                    print("training trainloader1!")
                    traindata = trainloader1
                elif lo == 1:
                    print("training trainloader2!")
                    traindata = trainloader2
                elif lo == 2:
                    print("training trainloader3!")
                    traindata = trainloader3
                elif lo == 3:
                    print("training trainloader4!")
                    traindata = trainloader4
                else:
                    print("training validloader!")
                    traindata = validloader
                labels = []
                train_label = []
                for batch in tqdm(traindata, desc='Training'):
                    if torch.cuda.is_available():
                        batch = {k: v.cuda() for k,v in batch.items()}
                    optimizer.zero_grad()
                    #print("batch length: ",(batch['input_ids'][0].size()))
                    output = self.model(**batch)
                    
                    pred = torch.argmax(output.logits, dim=-1)
                    labels.append(pred.long().item())
                    train_label.append((batch["labels"].long().item(), pred.long().item()))
                    self.clf_metrics.add_batch(predictions=pred.long(), references=batch["labels"].long())
                    #elf = self.clf_metrics.compute()
                    #print(f"{elf}")

                    output.loss.backward()
                    optimizer.step()
                    #if cur_step != 0 and cur_step % log_steps == 0:
                print(f"epoch: {ep}, cur_step: {cur_step}, loss: {output.loss.item()}")
                print('labels: ', labels)
                #print('train_label: ', train_label)
                    #cur_step += 1
                #acc = self.evalute(traindata)
                acc = self.clf_metrics.compute()
                print(f"epoch: {ep}, {acc}")
                end_time = time.time()
                run_time = end_time - start_time
                print(f"running: ", run_time, "seconds")
        torch.save(self.model, './longformer/longformer-epoch20.bin')

    def test(self):
        pass

    def predict(self):
        pass

    def collate_func(self, batch):
        texts, labels = [], []
        for item in batch:
            texts.append(item[0])
            labels.append(item[1])
        #inputs = self.tokenizer(texts, max_length=512, padding="max_length", truncation=True, return_tensors="pt")
        inputs = self.tokenizer(texts, max_length=2048, padding="max_length", truncation=True, return_tensors="pt")
        #print("inputs length: ", inputs['input_ids'][0].size())
        inputs["labels"] = torch.tensor(labels)
        return inputs