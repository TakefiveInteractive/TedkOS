#!/usr/bin/python2.7

import sys
import struct

'''
	magic ("\x01carray\x01file\x01")
	numOfObjs (uint32_t)
		For each obj:
		length (uint32_t) (length = 0 <=> padding)
		obj
		0x55 0xaa
'''

magic = "\x01carray\x01file\x01"

# elements in List will be converted to bytearray.
def makeCArray(inputList):
	ans = bytearray(magic)
	objList = [bytearray(x) for x in inputList]

	ans += struct.pack('<I', len(objList))

	for obj in objList:
		ans += struct.pack('<I', len(obj))
		ans += obj
		ans += bytearray(b'\x55')
		ans += bytearray(b'\xAA')

	return ans

if __name__ == "__main__":
	if len(sys.argv) == 1:
		print
		print "Usage: " + sys.argv[0] + " [output] [front_padding] (input0) (input1) ..."
		print "    If any input = '-', it represents a padding"
		print
		sys.exit(-1)
	outName = sys.argv[1]
	front_padding = int(sys.argv[2])
	inNames = sys.argv[3:]

	inObjects = [bytearray()] * front_padding
	for name in inNames:
		if name == '-':
			inObjects += [bytearray()]
		else:
			f = open(name, 'rb')
			inObjects += [f.read()]
			f.close()

	ans = makeCArray(inObjects)
	outFile = open(outName, 'wb+')
	outFile.write(ans)
	outFile.close()