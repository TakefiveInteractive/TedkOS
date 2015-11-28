#ifndef _X86_IDT_INIT_H
#define _X86_IDT_INIT_H

#include <inc/x86/desc.h>
#include <inc/klibs/lib.h>
#include <inc/klibs/spinlock.h>
#include <inc/klibs/function.h>
#include <inc/klibs/AutoSpinLock.h>
#include <boost/type_traits/function_traits.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/utility/enable_if.hpp>

// This function initializes IDT table,
// And loads the table to IDTR
extern void init_idt();

// Runs a block of code that must not be interrupted by NMI.
template <typename FuncType>
typename boost::disable_if_c<boost::is_same<typename boost::lambda_traits<FuncType>::result_type, void>::value, typename boost::lambda_traits<FuncType>::result_type>::type
runWithoutNMI(FuncType fn)
{
    spinlock_t nmi_lock = SPINLOCK_UNLOCKED;
    AutoSpinLock lock(&nmi_lock);

    // Disable NMI
    outb(inb(0x70) & 0x7F, 0x70);

    // Run our function
    typename boost::lambda_traits<FuncType>::result_type result = fn();

    // Enable NMI
    outb(inb(0x70) | 0x80, 0x70);

    return result;
}

template <typename FuncType>
typename boost::enable_if_c<boost::is_same<typename boost::lambda_traits<FuncType>::result_type, void>::value, void>::type
runWithoutNMI(FuncType fn)
{
    spinlock_t nmi_lock = SPINLOCK_UNLOCKED;
    AutoSpinLock lock(&nmi_lock);

    // Disable NMI
    outb(inb(0x70) & 0x7F, 0x70);

    // Run our function
    fn();

    // Enable NMI
    outb(inb(0x70) | 0x80, 0x70);
}

#ifdef __cplusplus
extern "C" {
#endif

// Get the number of non-syscall interrupt
//      that has NOT 'iret' yet.
// Useful for scheduler to determine:
//      whether it's at outmost interrupt.
extern int32_t num_nest_int();
extern spinlock_t num_nest_int_lock;
extern int32_t num_nest_int_val;

void interrupt_handler_with_number (size_t index);

#ifdef __cplusplus
}
#endif

#endif
