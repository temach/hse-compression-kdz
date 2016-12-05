import numpy
from pprint import pprint
import pdb
from glob import glob

pw = 50

predir = "./charts/"

axis_size_kb = "Размер в байтах"
axis_size_mb = "Размер в байтах"
axis_ops = "Колличество операций"
axis_ch = "Набор символов"

# choose in order
dounzip = "Разархивирование "
dozip = "Архивирование "

fsmall = "небольшие файлы "
fbig = "большие файлы "

chart_alg = "алгоритм {} "
chart_charset = "набор символов {} "
chart_size = "размер файла {} байт "


mbyte = 1 * 2**20
def is_small(i):
    return int(i) < mbyte

def get_small(d):
    return { k:[(x,y) for x,y in v if is_small(x)] for k,v in data.items()}
def get_big(d):
    return { k:[(x,y) for x,y in v if not is_small(x)] for k,v in data.items()}

#### ENCODING

def make_chart(chart_name, horiz, vert):
    return "Данные для графика: {}\nКомпонента Х: {}\nКомпонента У: {}\n".format(chart_name, horiz, vert)

def do_small_big(chart_name, horiz, vert, data):
### BEWARE THAT WE DONT ACTUALLY USE THE horiz PARAMETER
    # add small points
    chart = make_chart(chart_name + fsmall, axis_size_kb, axis_ops)
    small = get_small(data)
    with open(predir + chart_name + "-small.txt", "w") as f:
        print(chart, file=f)
        pprint(small, f, width=pw)

    # add big points
    chart = make_chart(chart_name + fbig, axis_size_mb, axis_ops)
    big = get_big(data)
    with open(predir + chart_name + "-big.txt", "w") as f:
        print(chart, file=f)
        pprint(big, f, width=pw)


##########

data_all = numpy.load("data_alg_all.npy").item()
for i in sorted( data_all):
    data = data_all[i]
    chart_name = (dozip + chart_alg).format(i)
    do_small_big(chart_name, "", axis_ops, data)

###
data_all = numpy.load("data_ch_all.npy").item()
for i in sorted( data_all):
    data = data_all[i]
    chart_name = (dozip + chart_charset).format(i)
    do_small_big(chart_name, "", axis_ops, data)


####
data_all = numpy.load("data_sz_all.npy").item()
for i in sorted( data_all):
    data = data_all[i]
    # do not make for small files
    if is_small(i):
        continue
    chart_name = (dozip + chart_size).format(i)
    chart = make_chart(chart_name, axis_ch, axis_ops)
    with open(predir + chart_name + ".txt", "w") as f:
        print(chart, file=f)
        pprint(data, f, width=pw)


#### DECODING

data_all = numpy.load("data_unz_alg_all.npy").item()
for i in sorted( data_all):
    data = data_all[i]
    chart_name = (dounzip + chart_alg).format(i)
    do_small_big(chart_name, "", axis_ops, data)

###
data_all = numpy.load("data_unz_ch_all.npy").item()
for i in sorted( data_all):
    data = data_all[i]
    chart_name = (dounzip + chart_charset).format(i)
    do_small_big(chart_name, "", axis_ops, data)


####
data_all = numpy.load("data_unz_sz_all.npy").item()
for i in sorted( data_all):
    data = data_all[i]
    # do not make for small files
    if is_small(i):
        continue
    chart_name = (dounzip + chart_size).format(i)
    chart = make_chart(chart_name, axis_ch, axis_ops)
    with open(predir + chart_name + ".txt", "w") as f:
        print(chart, file=f)
        pprint(data, f, width=pw)



## MAKE ONE BIG FILE
from os.path import isfile, join, abspath, basename
from os import listdir
import os
os.chdir(predir)

datapath = predir

other_files = sorted( glob("*.txt") )
with open("all_tables.txt", "w") as outfile:
    for fpath in other_files:
        with open(fpath, "r") as infile:
            x = infile.readlines()
            outfile.writelines(x)
        print("\n\n", file=outfile)

