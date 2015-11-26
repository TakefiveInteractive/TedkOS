#include <inc/drivers/kbterm.h>

namespace Term
{
    int32_t FOps::read(filesystem::File& fdEntity, uint8_t *buf, int32_t bytes)
    {
        return -EFOPS;
    }
    int32_t FOps::write(filesystem::File& fdEntity, const uint8_t *buf, int32_t bytes)
    {
        Term* t = getCurrentThreadInfo()->pcb.to_process->currTerm;

        // If this is not a terminal program, returns fault.
        if(!t)
            return -EFOPS;

        return t->write(fdEntity, buf, bytes);
    }
    int32_t FOps::open(filesystem::File& fdEntity)
    {
        Term* t = getCurrentThreadInfo()->pcb.to_process->currTerm;

        // If this is not a terminal program, returns fault.
        if(!t)
            return -EFOPS;
        else return 0;
    }
    int32_t FOps::close(filesystem::File& fdEntity)
    {
        Term* t = getCurrentThreadInfo()->pcb.to_process->currTerm;

        // If this is not a terminal program, returns fault.
        if(!t)
            return -EFOPS;
        else return 0;
    }
}

namespace KeyB
{
    int32_t FOps::read(filesystem::File& fdEntity, uint8_t *buf, int32_t bytes)
    {
        Term* t = getCurrentThreadInfo()->pcb.to_process->currTerm;

        // If this is not a terminal program, returns fault.
        if(!t)
            return -EFOPS;

        return t->read(fdEntity, buf, bytes);
    }
    int32_t FOps::write(filesystem::File& fdEntity, const uint8_t *buf, int32_t bytes)
    {
        return -1;
    }
    int32_t FOps::open(filesystem::File& fdEntity)
    {
        Term* t = getCurrentThreadInfo()->pcb.to_process->currTerm;

        // If this is not a terminal program, returns fault.
        if(!t)
            return -EFOPS;
        else return 0;
    }
    int32_t FOps::close(filesystem::File& fdEntity)
    {
        Term* t = getCurrentThreadInfo()->pcb.to_process->currTerm;

        // If this is not a terminal program, returns fault.
        if(!t)
            return -EFOPS;
        else
        {
            t->setOwner(-1);
            return 0;
        }
    }
}

