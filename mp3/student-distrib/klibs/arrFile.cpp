#include <inc/klibs/arrFile.h>
#include <inc/klibs/lib.h>

constexpr const char magic[] = "\1carray\1file\1";
constexpr const char ending[] = "\x55\xaa";

ArrFile::ArrFile() {
    _subArrLength = nullptr;
    _subArr = nullptr;
}

ArrFile* ArrFile::getInstance(char *ptr) {
    if (strncmp(magic, (const char *)ptr, sizeof(magic) - 1) != 0) return nullptr;
    ArrFile *obj = new ArrFile();

    ptr = ptr + (sizeof(magic) - 1);
    obj->_length = *((uint32_t *)ptr);

    printf("%d\n", obj->_length);

    obj->_subArrLength = new uint32_t[obj->_length];
    obj->_subArr = new void*[obj->_length];

    ptr++;
    for (uint32_t i = 0; i < obj->_length; i++) {
        obj->_subArrLength[i] = *((uint32_t *)ptr++);
        obj->_subArr[i] = ptr;
        if (obj->_subArrLength[i] == 0)
            obj->_subArr[i] = nullptr;
        ptr += obj->_subArrLength[i];
        if (strncmp(ending, (const char *)ptr, sizeof(ending) - 1) != 0) {
            obj->freeAll();
            return nullptr;
        }
        ptr += (sizeof(ending) - 1);
    }
    return obj;
}

ArrFile::~ArrFile() {
    freeAll();
}

ArrFile::ArrFile(const ArrFile &rhs) {
    copy(&rhs);
}

ArrFile& ArrFile::operator = (const ArrFile &rhs) {
    freeAll();
    copy(&rhs);
    return *this;
}

void* ArrFile::operator [] (size_t idx) const {
    if (idx > _length - 1) return nullptr;
    return _subArr[idx];
}

size_t ArrFile::length() const {
    return _length;
}

void ArrFile::copy(const ArrFile *rhs) {
    _subArrLength = new uint32_t[_length];
    _subArr = new void*[_length];
    memcpy(_subArrLength, rhs->_subArrLength, sizeof(size_t) * _length);
    memcpy(_subArr, rhs->_subArr, sizeof(void*) * _length);
}

void ArrFile::freeAll() {
    delete [] _subArrLength;
    delete [] _subArr;
}
