from os import listdir
from os.path import isfile, join, abspath, basename
import subprocess

datapath = abspath("./inputs")

def is_ops(f):
    return ".ops" in f

def is_txt(f):
    return ".txt" in f and not is_ops(f) and not is_haff(f) and not is_shan(f) and not is_unz_haff(f) and not is_unz_shan(f)

def is_haff(f):
    return ".haff" in f and not is_ops(f) and not is_txt(f)
def is_unz_haff(f):
    return "-unz-h.txt" in f and not is_ops(f)

def is_shan(f):
    return ".shan" in f and not is_ops(f) and not is_txt(f)
def is_unz_shan(f):
    return "-unz-s.txt" in f and not is_ops(f)

def get_file_list(condition):
    ret = []
    for f in listdir(datapath):
        p = join(datapath, f)
        if (isfile(p) and condition(f)):
            ret.append(p)
    return ret


# encode everything
textfiles = get_file_list(is_txt)
for f in textfiles:
    for enc in ["shennon", "huffman"]:
        x = subprocess.run(["./build-myprog/myprog", "-a", enc, "-i", f], stdout=subprocess.PIPE)

# decode what you encoded
def cond_encoded(f):
    return is_haff(f) or is_shan(f) and not is_txt(f)
encoded = get_file_list(cond_encoded)
for f in encoded:
    enc = "huffman"
    if is_shan(f):
        enc = "shennon"
    x = subprocess.run(["./build-myprog/myprog", "-a", enc, "-i", f], stdout=subprocess.PIPE)

# extra safety check
for f in textfiles:
    x1 = subprocess.run(["diff", f, f[:-4] + "-unz-s.txt"], stdout=subprocess.PIPE)
    if (x1.returncode > 0):
        print("bad encoding/decoding for shannon " + f + "\n" + str(x1.returncode) + "\n")
    x2 = subprocess.run(["diff", f, f[:-4] + "-unz-h.txt"], stdout=subprocess.PIPE)
    if (x2.returncode > 0):
        print("bad encoding/decoding for huffman " + f + "\n" + str(x2.returncode) + "\n")

