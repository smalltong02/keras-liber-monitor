import json
import os
import tempfile
import random
import string
from enum import Enum
from fastapi import FastAPI, HTTPException, Request
from fastapi.responses import JSONResponse
from albert.albertmodel import InnerAlbertInstance

global_predict_success = 0
global_predict_fail = 0

class ModelName(str, Enum):
    fasttext = "fasttext"
    gru = "gru"
    lstm = "lstm"
    bert = "bert"

def generate_random_string(length):
    return ''.join(random.choice(string.ascii_letters) for _ in range(length))

def write_string_to_tempfile(content):
    temp_dir = tempfile.gettempdir()
    random_filename = generate_random_string(10)
    temp_filepath = os.path.join(temp_dir, random_filename)
    print("temp file: ", temp_filepath)
    with open(temp_filepath, 'w') as temp_file:
        temp_file.write(content)
    return temp_filepath

app = FastAPI()

@app.get("/")
async def root():
    return {"message": "This is Classification Model Server"}

@app.get("/predict/{model_name}")
async def GetModel(model_name: ModelName):
    if model_name is ModelName.fasttext:
        return {"model": model_name, "message": "fasttext model is a FC model."}

    if model_name is ModelName.gru:
        return {"model": model_name, "message": "GRU model is a RNN model."}

    if model_name is ModelName.lstm:
        return {"model": model_name, "message": "LSTM model is a RNN model."}

    if model_name is ModelName.bert:
        return {"model": model_name, "message": "Bert model is a transformer model."}
    
    return {"model": model_name, "message": "Unsupported models"}

@app.post("/predict/{model_name}")
async def predict(model_name: ModelName, request: Request):
    global global_predict_success
    global global_predict_fail
    try:
        json_data = await request.json()
        features = json_data.get("features")
        
        if features is None:
            global_predict_fail += 1
            raise HTTPException(status_code=400, detail="Missing 'features' field in JSON")

        temp_filepath = write_string_to_tempfile(features)
        bertinst = InnerAlbertInstance("albert", temp_filepath, "", ".\\albert\\albert_model-ep61-loss-3.11e-9.pt", "")
        run_time, label, percentage = bertinst.predict()
        os.remove(temp_filepath)
        #print(run_time, label, percentage)
        data = {
            "message": "JSON processed successfully",
            "run_time": run_time,
            "label": label,
            "percentage": percentage,
        }
        global_predict_success += 1
        print("albert predict success count: ", global_predict_success, "  fail count: ", global_predict_fail)
        return JSONResponse(content=data)
    except json.JSONDecodeError:
        global_predict_fail += 1
        raise HTTPException(status_code=400, detail="Invalid JSON format")