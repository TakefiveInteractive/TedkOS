import png
import sys
import os

args = list(sys.argv)[1:]

def convert(arg):
    r = png.Reader(arg)
    bitmap = r.read()[2]
    colCount = 0
    whitespaceLine = []
    stdColLen = 0

    switcher = {
        "1": 20,
        "2": 26,
        "3": 40,
    }
    stdColLen = switcher[os.path.basename(arg)[0]]
    with open(arg + '.conv', 'wb') as f:
        for row in bitmap:
            if colCount == 0:
                colCount = len(row)
            whitespaceLine = row
            f.write(bytearray(row))
        if colCount < stdColLen:
            for i in range(stdColLen - colCount):
                f.write(bytearray(whitespaceLine))

for arg in args:
    convert(arg)
