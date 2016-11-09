import codecs

a = "a" * 45000
b = "b" * 13000
c = "c" * 12000
d = "d" * 16000
e = "e" *  9000
f = "f" *  5000
fin = a+b+c+d+e+f


with codecs.open("test.txt", "w", "utf-8") as f:
    f.write(fin)
