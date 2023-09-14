import os
import sys
import argparse
from longformer.longformermodel import InnerLongformerClassificationInstance

def main(args):
    if args.input is None or args.input == "":
        print("Input path is missing. Please provide --input with a valid path.")
        sys.exit(0)
    elif len(args.input) and os.path.exists(args.input) == False:
        print("Input path does not exist or is not a file, please provide the correct path.")
        sys.exit(0)
    if args.predict == True and (args.model_path is None or args.model_path == ""):
        print("model path must be specified when predicting.")
        sys.exit(0)
    if args.testing == True and (args.model_path is None or args.model_path == ""):
        print("model path must be specified when testing.")
        sys.exit(0)
    if args.testing == True and args.predict == True:
        print("testing and predicting are in conflict.")
        sys.exit(0)
    if args.testing == False and args.predict == False and (args.output is None or args.output == ""):
        print("Output path is missing. Please provide --output with a valid path.")
        sys.exit(0)
    if args.output is not None and (os.path.exists(args.output) == False or os.path.isdir(args.output) == False):
        print("Output path does not exist or is not a dir, please provide the correct path.")
        sys.exit(0)
    if args.model_path is not None and os.path.exists(args.model_path) == False:
        print("Model path does not exist or is not a file, please provide the correct path.")
        sys.exit(0)

    model = InnerLongformerClassificationInstance('longformer', args.input, args.output, None, args.model_path)
    if args.predict:
        model.predict()
    elif args.testing:
        model.test()
    else:
        model.train()
    #bertinst = InnerAlbertInstance("albert", args.input, args.output, args.model_path, args.dict_path)
    #if args.predict:
    #    bertinst.predict()
    #elif args.test:
    #    bertinst.test()
    #else:
    #    bertinst.train()

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Process options.")
    parser.add_argument("--input", type=str, required=True, help="dataset or predicting the path of sample.")
    parser.add_argument("--model_path", type=str, help="the path of modelbin.")
    parser.add_argument("--dict_path", type=str, help="the path of dictbin.")
    parser.add_argument("--predict", action="store_true", help="predicting the sample using the model.")
    parser.add_argument("--testing", action="store_true", help="test the samples using the model.")
    parser.add_argument("--output", type=str, help="output to path.")

    args = parser.parse_args()
    main(args)