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
    switcherWidth = {
        "1": 10,
        "2": 14,
        "3": 20,
    }
    stdColLen = switcher[os.path.basename(arg)[0]]
    stdWidth = switcherWidth[os.path.basename(arg)[0]]
    with open(arg + '.conv', 'wb') as f:
        numRow = 0
        for row in bitmap:
            extendedRow = list(row)
            extendedRow = extendedRow[1::2]
            if len(extendedRow) < stdWidth: extendedRow += [0]*(stdWidth - len(extendedRow))
            f.write(bytearray(extendedRow))
            numRow += 1
        while numRow < stdColLen:
            f.write(bytearray([0]*stdWidth))
            numRow += 1

for arg in args:
    convert(arg)
