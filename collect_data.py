import numpy

from os import listdir
import os
from os.path import isfile, join, abspath, basename
import itertools

datapath = abspath("./inputs")

# THIS IS OPERATIONS COLLECTOR FUNCTION
def get_all_sizes():
    def get_file_list(condition):
        ret = []
        for f in listdir(datapath):
            p = join(datapath, f)
            if (isfile(p) and condition(f)):
                ret.append(p)
        return ret

    all_sizes = {}

    # get all files
    allfiles = get_file_list(lambda f: True)
    for f in allfiles:
        sz, junk = basename(f).split('_', 1)
        text, number = sz.split('-')
        all_sizes[int(number)] = None

    return sorted( all_sizes.keys() )


def get_ops_under_condition(algo, size, charset):
    fname = "size-{}_charset-{}_sample-0.{}.ops".format(size, charset, algo)
    with open(datapath + os.sep + fname, "r") as opsfile:
        for ln in opsfile:
            # the ops count should be on the first line
            return int(ln)

def unz_get_ops_under_condition(algo, size, charset):
    fname = "size-{}_charset-{}_sample-0-unz-{}.txt.ops".format(size, charset, algo[0])
    with open(datapath + os.sep + fname, "r") as opsfile:
        for ln in opsfile:
            # the ops count should be on the first line
            return int(ln)


sizes = get_all_sizes()
sets = [0, 1, 2]
algos = ["haff", "shan"]

# ENCODING



# each algo+charset matches to size <-> ops
savedata = {}
for alg in algos:
    data = {}
    for ch in sets:
        key = ch
        data[ch] = []
        for sz in sizes:
            opcount = get_ops_under_condition(alg, sz, ch)
            data[key].append((sz, opcount))
    savedata[alg] = data
numpy.save('data_alg_all.npy', savedata)


# for each charset: both algo with size<->ops
savedata = {}
for i in sets:
    data_charset = {}
    for alg in algos:
        key = alg
        data_charset[key] = []
        for sz in sizes:
            opcount = get_ops_under_condition(alg, sz, i)
            data_charset[key].append((sz, opcount))
    savedata[i] = data_charset
numpy.save('data_ch_all.npy', savedata)


# for each size: both algo with charset<->ops
savedata = {}
for sz in sizes:
    data_sz = {}
    for alg in algos:
        key = alg
        data_sz[key] = []
        for ch in sets:
            opcount = get_ops_under_condition(alg, sz, ch)
            data_sz[key].append((ch, opcount))
    savedata[sz] = data_sz
numpy.save('data_sz_all.npy', savedata)



# DECODING

# each algo+charset matches to size <-> ops
savedata = {}
for alg in algos:
    data = {}
    for ch in sets:
        key = ch
        data[ch] = []
        for sz in sizes:
            opcount = unz_get_ops_under_condition(alg, sz, ch)
            data[key].append((sz, opcount))
    savedata[alg] = data
numpy.save('data_unz_alg_all.npy', savedata)


# for each charset: both algo with size<->ops
savedata = {}
for i in sets:
    data_charset = {}
    for alg in algos:
        key = alg
        data_charset[key] = []
        for sz in sizes:
            opcount = unz_get_ops_under_condition(alg, sz, ch)
            data_charset[key].append((sz, opcount))
    savedata[i] = data_charset
numpy.save('data_unz_ch_all.npy', savedata)


# for each size: both algo with charset<->ops
savedata = {}
for sz in sizes:
    data_sz = {}
    for alg in algos:
        key = alg
        data_sz[key] = []
        for ch in sets:
            opcount = unz_get_ops_under_condition(alg, sz, ch)
            data_sz[key].append((ch, opcount))
    savedata[sz] = data_sz
numpy.save('data_unz_sz_all.npy', savedata)
