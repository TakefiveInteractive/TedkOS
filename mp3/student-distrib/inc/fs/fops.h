#ifndef _FOPS_H
#define _FOPS_H

#include <stddef.h>
#include <stdint.h>
#include <inc/klibs/maybe.h>

typedef void FsSpecificData;

class IFOps
{
public:
    virtual ~IFOps() = 0;
    virtual int32_t read(FsSpecificData *fdData, uint8_t *buf, int32_t bytes) = 0;
    virtual int32_t write(FsSpecificData *fdData, const uint8_t *buf, int32_t bytes) = 0;
};

typedef Maybe<IFOps *> (* FOpsGetter) ();

#endif /* _FOPS_H */
