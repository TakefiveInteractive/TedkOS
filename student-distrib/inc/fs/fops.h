#ifndef _FOPS_H
#define _FOPS_H

#include <stddef.h>
#include <stdint.h>
#include <inc/klibs/maybe.h>

typedef void FsSpecificData;

class IFOps
{
public:
    virtual ~IFOps() {}
    virtual int32_t read(FsSpecificData *fdData, uint8_t *buf, int32_t bytes) = 0;
    virtual int32_t write(FsSpecificData *fdData, const uint8_t *buf, int32_t bytes) = 0;

    // If this device supports seeking. Default to false.
    virtual bool canSeek() { return false; }
    // This is called whenever a seek happens
    virtual void seekCallback(uint32_t newOffset) { };
};

typedef Maybe<IFOps *> (* FOpsGetter) ();

#endif /* _FOPS_H */
