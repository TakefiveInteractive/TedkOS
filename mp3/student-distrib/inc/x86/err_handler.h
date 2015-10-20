#ifndef _ERR_HANDLER_H_
#define _ERR_HANDLER_H_

/*
 * This function is invoked by the common interrupt function when an 'error' occurs.
 */
void __attribute__((fastcall)) exception_handler(size_t, unsigned long int);

#ifdef __cplusplus
extern "C" {
#endif
    extern const unsigned long int ErrorCodeInExceptionBitField;
#ifdef __cplusplus
}
#endif


#endif

