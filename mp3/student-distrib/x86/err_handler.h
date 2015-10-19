#ifndef _ERR_HANDLER_H_
#define _ERR_HANDLER_H_

/*
 * This function is invoked directly by the CPU when an 'error' occurs.
 * It uses assembly linkage and pops off the error code.
 */
void raw_exception_handler(void);

#endif

