#ifndef _KERNEL_ERRORS_H
#define _KERNEL_ERRORS_H

// Use these just like the linux ones: 'return -ENOMEM;'
// return 0 if success.

// EFOPS: Required by handout, general error code for FOPS
#define EFOPS               1
#define ENOMEM              2
#define EINVAL              3

#endif /* _KERNEL_ERRORS_H */
