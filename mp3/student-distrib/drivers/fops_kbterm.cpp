#include <inc/drivers/kbterm.h>
#include <inc/proc/sched.h>
#include <inc/proc/tasks.h>

namespace Term
{
    int32_t FOps::read(FsSpecificData* fdData, uint8_t *buf, int32_t bytes)
    {
        return -EFOPS;
    }
    int32_t FOps::write(FsSpecificData* fdData, const uint8_t *buf, int32_t bytes)
    {
        Term* t = getCurrentThreadInfo()->pcb.to_process->currTerm;

        // If this is not a terminal program, returns fault.
        if(!t)
            return -EFOPS;

        return t->write(fdData, buf, bytes);
    }
    FOps::FOps()
    {}
    Maybe<IFOps*> FOps::getNewInstance()
    {
        Term* t = getCurrentThreadInfo()->pcb.to_process->currTerm;

        // If this is not a terminal program, returns fault.
        if(!t)
            return Nothing;
        else return reinterpret_cast<IFOps*>(new FOps);
    }
    FOps::~FOps()
    {}
}

namespace KeyB
{
    int32_t FOps::read(FsSpecificData* fdData, uint8_t *buf, int32_t bytes)
    {
        Term::Term* t = getCurrentThreadInfo()->pcb.to_process->currTerm;

        // If this is not a terminal program, returns fault.
        if(!t)
            return -EFOPS;

        return t->read(fdData, buf, bytes);
    }
    int32_t FOps::write(FsSpecificData* fdData, const uint8_t *buf, int32_t bytes)
    {
        return -1;
    }
    FOps::FOps()
    {}
    Maybe<IFOps*> FOps::getNewInstance()
    {
        Term::Term* t = getCurrentThreadInfo()->pcb.to_process->currTerm;

        // If this is not a terminal program, returns fault.
        if(!t)
            return Nothing;
        else return reinterpret_cast<IFOps*>(new FOps);
    }
    FOps::~FOps()
    {
        Term::Term* t = getCurrentThreadInfo()->pcb.to_process->currTerm;

        // If this is not a terminal program, returns fault.
        if(!t)
            printf("Unexpected KeyB::FOps::close error at %s\n", __FILE__);
        else
            t->setOwner(-1);
    }
}

