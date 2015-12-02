import png
import sys
import os

args = list(sys.argv)[1:]

def convert(arg):
    r = png.Reader(arg)
    bitmap = r.read()[2]
    stdColLen = 0

    switcher = {
        "1": 20,
        "2": 28,
        "3": 42,
    }
    stdColLen = switcher[os.path.basename(arg)[0]]
    with open(arg + '.conv', 'wb') as f:
        for row in bitmap:
            f.write(bytearray(row))
            if len(row) < stdColLen:
                for i in range((stdColLen - len(row))):
                    f.write(bytearray([0]))

for arg in args:
    convert(arg)
