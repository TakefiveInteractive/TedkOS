#!/usr/bin/python2.7

import sys

'''
magic ("\x01carray\x01file\x01")
numOfObjs (uint32_t)
For each obj:
    length (uint32_t) (length = 0 <=> padding)
    obj
    0x55 0xaa
'''

magic = "\x01carray\x01file\x01"


def makeFakeArrFile():
    fileArr = bytearray(magic)
    fileArr += bytearray(b'\x03')
    fileArr += bytearray(b'\x00')
    fileArr += bytearray(b'\x00')
    fileArr += bytearray(b'\x00')

    fileArr += bytearray(b'\x02')
    fileArr += bytearray(b'\x00')
    fileArr += bytearray(b'\x00')
    fileArr += bytearray(b'\x00')
    fileArr += bytearray(b'\x01')
    fileArr += bytearray(b'\x02')
    fileArr += bytearray(b'\x55')
    fileArr += bytearray(b'\xAA')

    fileArr += bytearray(b'\x02')
    fileArr += bytearray(b'\x00')
    fileArr += bytearray(b'\x00')
    fileArr += bytearray(b'\x00')
    fileArr += bytearray(b'\x03')
    fileArr += bytearray(b'\x04')
    fileArr += bytearray(b'\x55')
    fileArr += bytearray(b'\xAA')

    fileArr += bytearray(b'\x00')
    fileArr += bytearray(b'\x00')
    fileArr += bytearray(b'\x00')
    fileArr += bytearray(b'\x00')
    fileArr += bytearray(b'\x55')
    fileArr += bytearray(b'\xAA')

    return fileArr

if __name__ == "__main__":
    fileArr = makeFakeArrFile()
    outName = "testArrFile"
    outFile = open(outName, 'wb+')
    outFile.write(fileArr)
    outFile.close()
