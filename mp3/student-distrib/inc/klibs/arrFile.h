#ifndef _ARRFILE_
#define _ARRFILE_

#include <stdint.h>
#include <stddef.h>
/*
 magic ("\x01carray\x01file\x01")
 numOfObjs (uint32_t)
     For each obj:
     length (uint32_t) (length = 0 <=> padding)
     obj
     0x55 0xaa

--------
 \x01carray\x01file\x01
 \x03\x00\x00\x00 - arr length

 \x08\x00\x00\x00 - sizeof(arr[0])
 \x12\xaf\x4f\x00\x00\x00\x00\x00 - void*
 \x55\xaa

 \00000000
 \x55\xaa

 \x01\000000
 \x11
 \x55\xaa
 */

class ArrFile {
private:
    size_t _length;
    size_t* _subArrLength;
    void** _subArr;

    void copy(const ArrFile *rhs);
    void freeAll();
    ArrFile();
public:
    static ArrFile* getInstance(char *ptr);
    ~ArrFile();
    ArrFile(const ArrFile &rhs);
    ArrFile& operator = (const ArrFile &rhs);
    void* operator [] (size_t idx) const;
    size_t length() const;
};

#endif
