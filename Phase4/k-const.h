#ifndef __K_CONST__
#define __K_CONST__

// Used when none by running process
#define NONE -1
// Timer INTR constant identifier
#define TIMER_INTR 32
// Programmable Interrupt Controller(PIC) I/O
#define PIC_MASK 0x21
// Mask for PIC
#define MASK ~0x01
// PIC I/O
#define PIC_CONTROL 0x20
// Sent to PIC when timer service done
#define TIMER_DONE 0x60

// Handly loop limit exec asm("inb $0x80");
#define LOOP 1666666
// Max timer count, then rotate process
#define TIME_SLICE 310
// Max number of processes
#define PROC_SIZE 20
// Proccess stack in bytes
#define PROC_STACK_SIZE 4096
// Queueing capacity
#define Q_SIZE 20

// Foreground bright white, black background
#define VID_MASK 0x0f00
// Home position, upper-left corener
#define VID_HOME (unsigned short *)0xb8000

// Interupt table values
#define GETPID_CALL 48
#define SHOWCHAR_CALL 49
#define SLEEP_CALL 50

#define MUX_CREATE_CALL 51
#define MUX_OP_CALL 52
#define MUX_SIZE 20
#define STR_SIZE 101
#define LOCK 1
#define UNLOCK 2
#define STDOUT 1

#endif
