#include <inc/x86/filesystem.h>

namespace filesystem {

uint32_t fsceil(uint32_t len, uint32_t blkSize)
{
    uint32_t remainder = len % blkSize;
    if (remainder > 0) return len / blkSize + 1;
    return len / blkSize;
}

struct __attribute__ ((__packed__)) name_tmp { char name[MaxFilenameLength]; };

void KissFS::initFromMemoryAddress(uint8_t *startingAddr, uint8_t *endingAddr)
{
    this->imageStartingAddress = startingAddr;
    Reader reader(startingAddr);
    // Read boot block
    reader >> numDentries >> numInodes >> numTotalDataBlocks >> Reader::skip<52>();
    for (size_t i = 0; i < numDentries; i++)
    {
        name_tmp ntmp;
        reader >> ntmp >> dentries[i].filetype >> dentries[i].inode >> Reader::skip<24>();
        for (size_t j = 0; j < MaxFilenameLength; j++)
        {
            dentries[i].filename[j] = ntmp.name[j];
        }
    }
    // Read inodes
    for (size_t i = 0; i < numInodes; i++)
    {
        reader.reposition(BlockSize * (i + 1));
        reader >> inodes[i].size;
        inodes[i].numDataBlocks = fsceil(inodes[i].size, BlockSize);
        for (size_t j = 0; j < inodes[i].numDataBlocks; j++)
        {
             reader >> inodes[i].datablocks[j];
        }
    }

    numBlocks = (size_t)(endingAddr - startingAddr) / BlockSize;

    // Initialize hash table
    for (size_t i = 0; i < numDentries; i++)
    {
        dentryIndexOfFilename.put(Filename(dentries[i].filename), i);
    }
}

int32_t KissFS::readDentry(const uint8_t* fname, dentry_t* dentry)
{
    Filename fn((const char*)fname);
    bool found;
    uint32_t dentryIdx = dentryIndexOfFilename.get(fn, found);
    if (!found)
    {
        return -1;
    }
    else
    {
        *dentry = dentries[dentryIdx];
        return 0;
    }
}

int32_t KissFS::readDentry(uint32_t index, dentry_t* dentry)
{
    if (index <= numDentries)
    {
        *dentry = dentries[index];
        return 0;
    }
    else
    {
        return -1;
    }
}

int32_t KissFS::readData(uint32_t inode, uint32_t offset, uint8_t *buf, uint32_t length)
{
    uint32_t read = 0;
    if (inode <= numInodes)
    {
        uint32_t startingDataBlock = offset / BlockSize;
        uint32_t bytesRemaining = inodes[inode].size - offset;
        uint32_t realOffset = offset % BlockSize;
        for (size_t i = startingDataBlock; i < inodes[inode].numDataBlocks; i++)
        {
            uint32_t datablockId = inodes[inode].datablocks[i];
            if (datablockId <= numTotalDataBlocks)
            {
                // read stuff
                uint32_t len = length;
                if (len > bytesRemaining) len = bytesRemaining;
                if (length > BlockSize) len = BlockSize;
                if (!readBlock(datablockId, realOffset, buf + read, len)) return -1;
                length -= len;
                bytesRemaining -= len;
                read += len;
                // Start from zero in next block
                realOffset = 0;
                if (length <= 0) break;
            }
            else
            {
                return -1;
            }
        }
    }
    else
    {
        return -1;
    }
    return read;
}

bool KissFS::readBlock(uint32_t datablockId, uint32_t offset, uint8_t *buf, uint32_t len)
{
    uint32_t rawBlockId = datablockId + numInodes + 1;
    if (rawBlockId >= numBlocks) return false;

    memcpy(buf, imageStartingAddress + rawBlockId * BlockSize + offset, len);
    return true;
}

KissFS kissFS;

}
