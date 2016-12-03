import codecs
import itertools
import random

def data_generator(size, chars):
    return (random.choice(chars) for _ in range(size))

kibibyte = 2**10
mebibyte = 2**20

sizes = [
    20*kibibyte
    , 40*kibibyte
    , 60*kibibyte
    # , 80*kibibyte
    # , 100*kibibyte
    # , 1*mebibyte
    # , 2*mebibyte
    # , 3*mebibyte
    ]

# the first set also contains the SPACE character
set1 = "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz"
set2 = set1 + "АаБбВвГгДдЕеЁёЖжЗзИиЙйКкЛлМмНнОоПпРрСсТтУуФфХхЦцЧчШшЩщЪъЫыЬьЭэЮюЯя"
set3 = set2 + "+-*/=.,;:?!%@#$&~()[]{}<>\"\'"
sets = [set1, set2, set3]

samples = [0, 1, 2]


x = list(itertools.product(sizes, sets, samples))
for sz, chars, smple in x:
    fname = "size-{0}_charset-{1}_sample-{2}".format(sizes.index(sz), sets.index(chars), smple)
    fullname = "./inputs/{}".format(fname)
    with codecs.open(fullname, "w", "utf-8") as f:
        f.write(''.join(data_generator(sz, chars)))

