#include <inc/fs/kiss.h>
#include <inc/mbi_info.h>
#include <inc/klibs/memory.h>
#include <inc/x86/err_handler.h>
#include <inc/klibs/palloc.h>
#include <boost/unique_ptr.hpp>
#include <inc/x86/paging.h>

using boost::unique_ptr;
using memory::operator "" _MB;

namespace filesystem {

static const int SPECIAL_DEVICE = 0;
static const int DIRECTORY = 1;
static const int NORMAL_FILE = 2;

void KissFS::init()
{
    module_t* mod = (module_t*) MultiBootInfoAddress->mods_addr;
    initFromMemoryAddress((uint8_t *) mod->mod_start, (uint8_t *) mod->mod_end);
}

bool KissFS::open(const char* filename, FsSpecificData *data)
{
    Filename fn((const char*)filename);
    bool found;
    uint32_t dentryIdx = dentryIndexOfFilename.get(fn, found);
    if (!found)
    {
        return false;
    }
    else
    {
        data->filetype = dentries[dentryIdx].filetype;
        if (dentries[dentryIdx].filetype == DIRECTORY)
        {
            // Dir
            data->dentryData.base = reinterpret_cast<uint8_t *>(&dentries[0]);
            data->dentryData.idx = 0;
            data->dentryData.max = numDentries;
        }
        else
        {
            // File
            data->inode = dentries[dentryIdx].inode;
        }
        return true;
    }
}

int32_t KissFS::read(FsSpecificData *data, uint32_t offset, uint8_t *buf, uint32_t len)
{
    if (data->filetype == DIRECTORY)
    {
        return readDir(data, offset, buf, len);
    }
    else
    {
        return readData(data->inode, offset, buf, len);
    }
}

int32_t KissFS::readDir(FsSpecificData *data, uint32_t offset, uint8_t *buf, uint32_t len)
{
    // Read directory
    dentry_t *dentries = reinterpret_cast<dentry_t *>(data->dentryData.base);
    if (data->dentryData.idx >= data->dentryData.max)
    {
        return -1;
    }
    else
    {
        strncpy(reinterpret_cast<char *>(buf), dentries[data->dentryData.idx].filename, len);
        data->dentryData.idx++;
        if (data->dentryData.idx == data->dentryData.max - 1) return 0;
        return strlen(reinterpret_cast<char *>(buf));
    }
}

int32_t KissFS::write(FsSpecificData *data, uint32_t offset, const uint8_t *buf, uint32_t len)
{
    // Read-only
    return -1;
}

bool KissFS::close(FsSpecificData *fdData)
{
    // TODO: clean up memory
    return true;
}

struct __attribute__ ((__packed__)) name_tmp { char name[MaxFilenameLength]; };

void KissFS::initFromMemoryAddress(uint8_t *startingAddr, uint8_t *endingAddr)
{
    // map the module to virtual addresses
    uint32_t numPages = memory::ceil((uint32_t)(endingAddr - startingAddr), 4_MB);
    unique_ptr<uint16_t[]> physPageIndices(new uint16_t[numPages]);
    auto virtAddr = palloc::virtLast1G.allocConsPage(numPages, true);
    if (virtAddr)
    {
        // Establish mapping
        for (size_t i = 0; i < numPages; i++)
        {
            palloc::cpu0_memmap.addCommonPage(
                    palloc::VirtAddr((uint8_t *)(+virtAddr) + i * 4_MB),
                    palloc::PhysAddr((uint32_t)(startingAddr + i * 4_MB) >> 22, PG_WRITABLE));
        }
        RELOAD_CR3();
    }
    else
    {
        trigger_exception<27>();
    }

    this->imageStartingAddress = (uint8_t *) +virtAddr + ((uint32_t)startingAddr - ((uint32_t)startingAddr & ALIGN_4MB_ADDR));
    Reader reader(this->imageStartingAddress);
    // Read boot block
    reader >> numDentries >> numInodes >> numTotalDataBlocks >> Reader::skip<52>();
    if (numDentries > MaxNumFiles) numDentries = MaxNumFiles;
    dentries = new dentry_t[numDentries];
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
    if (numInodes > MaxNumFiles) numInodes = MaxNumFiles;
    inodes = new inode_t[numInodes];
    for (size_t i = 0; i < numInodes; i++)
    {
        reader.reposition(BlockSize * (i + 1));
        reader >> inodes[i].size;
        inodes[i].numDataBlocks = memory::ceil(inodes[i].size, BlockSize);
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
                if (len > BlockSize) len = BlockSize;
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

}
