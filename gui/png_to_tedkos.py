import png
import sys

args = list(sys.argv)[1:]

def convert(arg):
    r = png.Reader(arg)
    bitmap = r.read()[2]
    with open(arg + '.conv', 'wb') as f:
        for row in bitmap:
            f.write(bytearray(row))

for arg in args:
    convert(arg)
