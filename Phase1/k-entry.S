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
    // return from interupt
    iret

ENTRY(TimerEntry)
    // push all (8) registers
    pusha
    mov %esp, %ebx
    movw $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    leal k_stack + K_STACK_SIZE, %esp
    push %ebx
    // clear the direction flag of efl
    cld
    // call C function Kernal
    call CNAME(Kernal)
