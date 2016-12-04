from os import listdir
from os.path import isfile, join
import subprocess

def is_ops(s):
    return ".ops" in f

def is_txt(s):
    return ".txt" in f and not is_ops(f)

def is_haff(s):
    return ".haff" in f and not is_ops(f)
def is_unz_haff(s):
    return "-unz-h.txt" in f and not is_ops(f)

def is_shan(s):
    return ".shan" in f and not is_ops(f)
def is_unz_shan(s):
    return "-unz-s.txt" in f and not is_ops(f)


datapath = "./inputs"
textfiles = [f for f in listdir(datapath) if isfile(join(datapath, f)) and is_txt(f)]

for f in onlyfiles:
    for enc in ["shennon", "huffman"]:
        subprocess.run(["./build-myprog/myprog", "-a", enc, "-i", f], stdout=subprocess.PIPE)

encoded = [f for f in listdir(datapath) if isfile(join(datapath, f)) and (is_haff(f) or is_shan(f))]
for f in encoded:
    enc = "huffman"
    if (is_shan(f):
        enc = "shennon"
    subprocess.run(["./build-myprog/myprog", "-a", enc, "-i", f], stdout=subprocess.PIPE)

for f in textfiles:
    x1 = subprocess.run(["diff", f, f + "-unz-s.txt"], stdout=subprocess.PIPE)
    x2 = subprocess.run(["diff", f, f + "-unz-h.txt"], stdout=subprocess.PIPE)
    if (x1.returncode > 0 or x2.returncode > 0):
        print("bad encoding/decoding for shannon/huffman " + f + "\n" + x1 + "\n" + x2)

