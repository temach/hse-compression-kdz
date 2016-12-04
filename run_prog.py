from os import listdir
from os.path import isfile, join, abspath, basename
import subprocess
import pdb

def is_ops(f):
    return ".ops" in f

def is_txt(f):
    return ".txt" in f and not is_ops(f)

def is_haff(f):
    return ".haff" in f and not is_ops(f)
def is_unz_haff(f):
    return "-unz-h.txt" in f and not is_ops(f)

def is_shan(f):
    return ".shan" in f and not is_ops(f)
def is_unz_shan(f):
    return "-unz-s.txt" in f and not is_ops(f)


datapath = abspath("./inputs")

textfiles = []
for f in listdir(datapath):
    p = join(datapath, f)
    if (isfile(p) and is_txt(f)):
        textfiles.append(p)

for f in textfiles:
    for enc in ["shennon", "huffman"]:
        x = subprocess.run(["./build-myprog/myprog", "-a", enc, "-i", f], stdout=subprocess.PIPE)

encoded = [f for f in listdir(datapath) if isfile(join(datapath, f)) and (is_haff(f) or is_shan(f))]
for f in encoded:
    enc = "huffman"
    if is_shan(f):
        enc = "shennon"
    x = subprocess.run(["./build-myprog/myprog", "-a", enc, "-i", f], stdout=subprocess.PIPE)

for f in textfiles:
    x1 = subprocess.run(["diff", f, f + "-unz-s.txt"], stdout=subprocess.PIPE)
    x2 = subprocess.run(["diff", f, f + "-unz-h.txt"], stdout=subprocess.PIPE)
    if (x1.returncode > 0 or x2.returncode > 0):
        print("bad encoding/decoding for shannon/huffman " + f + "\n" + x1 + "\n" + x2)

