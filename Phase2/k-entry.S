#include "k-const.h"
#include <spede/machine/asmacros.h>
#define K_STACK_SIZE (PROC_STACK_SIZE * 8)

.comm k_stack, K_STACK_SIZE, 1
.text

ENTRY(Loader)
    // copy 4 bytes from 4B above esp to ebx
    mov 4(%esp), %ebx
    // copy ebx to esp (point esp to process trapframe)
    mov %ebx, %esp
    // pop all (8) registers
    popa
    //incr esp by 4 (add $4 to %esp -- skip 'entry_id' in trapframe) <-- !!! 
    add $4, %esp
    // return from interupt
    iret

ENTRY(TimerEntry)
    //(new constant for a sys call)
    pushl $TIMER_INTR
    jmp PreKernel

ENTRY(GetPidEntry)
    //(new constant for a sys call)
    pushl $GETPID_CALL
    jmp PreKernel

ENTRY(ShowCharEntry)
    //(new constant for a sys call)
    pushl $SHOWCHAR_CALL
    jmp PreKernel

ENTRY(SleepEntry)
    //(new constant for a sys call)
    pushl $SLEEP_CALL
    jmp PreKernel

// build rest of trapframe, switch stack, pass arg, call Kernel
PreKernel: 
    // push all: eax ecx edx ebx esp ebp esi edi
    pusha   
    movl %esp, %ebx          // copy esp
    movw $0x10, %ax          // put 16 to ax
    mov %ax, %ds             // set kernel data segment registers ds
    mov %ax, %es             // set kernel data segment registers es
    leal k_stack + K_STACK_SIZE, %esp  // set esp to top of kernel stack
    pushl %ebx               // push (proc TF addr) there
    cld                      // clear direction flag bit
    call CNAME(Kernel)       // call Kernel code
