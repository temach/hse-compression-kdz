import numpy
import leather

predir = "./charts/"

axis_size_kb = "Размер Кб"
axis_size_mb = "Размер Мб"
axis_ops = "Колличество операций (в тысячах)"
axis_ch = "Набор символов"

# choose in order
dounzip = "Разархивирование "
dozip = "Архивирование "

fsmall = "небольшие файлы "
fbig = "большие файлы "

chart_alg = "алгоритм {} "
chart_charset = "набор символов {} "
chart_size = "размер файла {} байт "

leather.theme.default_chart_height = 600
leather.theme.default_chart_width = 600


mbyte = 1 * 2**20
def is_small(i):
    return int(i) < mbyte

def get_small(d):
    return { k:[(x,y) for x,y in v if is_small(x)] for k,v in data.items()}
def get_big(d):
    return { k:[(x,y) for x,y in v if not is_small(x)] for k,v in data.items()}

#### ENCODING

def make_chart(chart_name, horiz, vert):
    chart = leather.Chart(chart_name)
    axis_x = leather.Axis(name=horiz)
    axis_y = leather.Axis(name=vert)
    chart.set_x_axis(axis_x)
    chart.set_y_axis(axis_y)
    return chart

def do_small_big(chart_name, horiz, vert, data):
### BEWARE THAT WE DONT ACTUALLY USE THE horiz PARAMETER
    # add small points
    chart = make_chart(chart_name + fsmall, axis_size_kb, axis_ops)
    small = get_small(data)
    for algoname, points in small.items():
        chart.add_line([(int(p[0])/1000, int(p[1])/1000) for p in points], name=algoname)
    chart.to_svg(predir + "{}-small.png".format(chart_name))

    # add big points
    chart = make_chart(chart_name + fbig, axis_size_mb, axis_ops)
    big = get_big(data)
    for algoname, points in big.items():
        chart.add_line([(int(p[0])/1000000, int(p[1])/1000) for p in points], name=algoname)
    chart.to_svg(predir + "{}-big.png".format(chart_name))


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
    for algoname, points in data.items():
        chart.add_line([(int(p[0]), int(p[1])/1000) for p in points], name=algoname)
    chart.to_svg(predir + "{}.png".format(chart_name))


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
    for algoname, points in data.items():
        chart.add_line([(int(p[0]), int(p[1])/1000) for p in points], name=algoname)
    chart.to_svg(predir + "{}.png".format(chart_name))


#### Collect ALL graphs in one pdf file all_graphs.pdf
import subprocess

from glob import glob
import os
os.chdir(predir)

pdfout = "all_charts.pdf"

cmd = "rsvg-convert -f pdf -o ".split()
cmd.append(pdfout)
for i in sorted( glob("*.png")):
    cmd.append("'{}'".format(i))

os.system(' '.join(cmd))

