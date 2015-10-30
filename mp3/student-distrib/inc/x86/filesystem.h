#ifndef _FILESYSTEM_H_
#define _FILESYSTEM_H_

#include <stddef.h>
#include <stdint.h>
#include <inc/multiboot.h>
#include <inc/lphashtable.h>
#include <inc/lib.h>

struct dentry_t {
    char filename[33] = {};
    uint32_t filetype;
    uint32_t inode;
};

struct inode_t {
    uint32_t size;
    uint32_t datablocks[4096 / sizeof(uint32_t) - 1];
    uint32_t numDataBlocks;
};

#ifdef __cplusplus
namespace filesystem {

    static const uint32_t MaxFilenameLength = 32;
    static const uint32_t MaxNumFiles = 64;
    static const uint32_t BlockSize = 4096;

    /* Class holding a filename that implements a proper equal operator */
    class Filename {
        private:
            const char *str;
        public:
            Filename(const char *s) { this->str = s; }

            Filename() { this->str = nullptr; }

            bool operator == (const Filename& rhs) const {
                return strncmp(str, rhs.str, MaxFilenameLength) == 0;
            }

            bool operator != (const Filename& rhs) const {
                return !(*this == rhs);
            }

            const char operator [] (size_t idx) const { return str[idx]; };
    };

    struct HashFunc {
        static uint32_t hash(const Filename& s)
        {
            uint32_t hash = 0;
            uint32_t used = 0;
            while (s[used] && used < MaxFilenameLength)
            {
                hash = hash * 101 + s[used];
                used++;
            }
            return hash;
        }
    };

    template<size_t num>
    struct __attribute__ ((__packed__)) SkipStruct {
        uint8_t members[num];
        // Assignment does nothing!
        const SkipStruct& operator= (const SkipStruct &source) const
        {
            return *this;
        }
    };

    class Reader {
        private:
            uint8_t *startingAddr;
            size_t read;

        public:

            Reader(uint8_t *startingAddr)
            {
                this->startingAddr = startingAddr;
                this->read = 0;
            }

            template<typename T>
            Reader& operator >> (T& val)
            {
                T local = T();
                const size_t size = sizeof(T);
                uint8_t *a = (uint8_t *) &local;
                for (size_t i = 0; i < size; i++)
                {
                    a[i] = startingAddr[read];
                    read++;
                }
                val = local;
                return *this;
            }

            template<size_t num>
            static const SkipStruct<num> skip()
            {
                return SkipStruct<num>();
            }

            void reposition(size_t offset)
            {
                read = offset;
            }
    };

    class KissFS {
    private:
        /* The hash table size must be not smaller than the number of dentries */
        util::LinearProbingHashTable<133, Filename, uint32_t, HashFunc> dentryIndexOfFilename;
        dentry_t dentries[MaxNumFiles];
        inode_t inodes[MaxNumFiles];
        uint32_t numDentries;
        uint32_t numInodes;
        uint32_t numTotalDataBlocks;
        uint8_t *imageStartingAddress;
        uint8_t imageLength;
        uint32_t numBlocks;

        bool readBlock(uint32_t datablockId, uint32_t offset, uint8_t *buf, uint32_t len);

    public:
        void initFromMemoryAddress(uint8_t *mem, uint8_t *end);
        int32_t readDentry(const uint8_t* fname, dentry_t* dentry);
        int32_t readDentry(uint32_t index, dentry_t* dentry);
        int32_t readData(uint32_t inode, uint32_t offset, uint8_t *buf, uint32_t length);
    };

    /* Singleton object here */
    extern KissFS kissFS;
}

#endif

#endif
