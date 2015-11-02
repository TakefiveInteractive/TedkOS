#ifndef _FOPS_H
#define _FOPS_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* General RETURN VALUE policy:
    0 means success for Open and Close.
    negative means error for all functions.
    For read, write, non-negative numbers are number of bytes processed.
*/

/* Read from the device */
typedef int32_t (*FOpsReadImpl) (void* fdEntity, uint8_t *buf, int32_t bytes);

/* Write to device */
typedef int32_t (*FOpsWriteImpl) (void* fdEntity, const uint8_t *buf, int32_t bytes);

/* initialize the device */
// fdEntity is pointer to the structure at fd index.
//   the structure should save some general information shared
//   between current process and this device.
// currently this structure type is NOT decided yet.
typedef int32_t (*FOpsOpenImpl) (void* fdEntity);

/* clean up after closing the device */
// fdEntity is pointer to the structure at fd index.
//   the structure should save some general information shared
//   between current process and this device.
// currently this structure type is NOT decided yet.
typedef int32_t (*FOpsCloseImpl) (void* fdEntity);

typedef struct {
    FOpsOpenImpl open;
    FOpsCloseImpl close;
    FOpsReadImpl read;
    FOpsWriteImpl write;
} FOpsTable;

#ifdef __cplusplus
}
#endif

#endif /* _FOPS_H */
