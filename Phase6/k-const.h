#ifndef __K_CONST__
#define __K_CONST__

// Used when none by running process
#define NONE -1
// Timer INTR constant identifier
#define TIMER_INTR 32
// Programmable Interrupt Controller(PIC) I/O
#define PIC_MASK 0x21
// Mask for PIC
// #define MASK ~0x01
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

// p4, 2 terminals
#define TERM_SIZE 2
// 1st terminal intr ID
#define TERM0_INTR 35
// 2nd terminal intr ID
#define TERM1_INTR 36
// I/O base of 1st term 0010 1111 1000
#define TERM0_IO_BASE 0x2f8
// I/O base of 2nd term 0011 1110 1000
#define TERM1_IO_BASE 0x3e8
// to send to PIC when 1st term served
#define TERM0_DONE_VAL 0x63
// to send to PIC when 2st term served
#define TERM1_DONE_VAL 0x64
// terminal has a Transmit Ready event
#define TXRDY 2
// terminal has a Receive Ready event
#define RXRDY 4
// new mask 1111_1111_1111_1111_1111_1111_1110_0110
#define MASK 0xffffffe6
#define TRUE 1
#define FALSE 0

// Phase 6
#define FORK_CALL 53
#define WAIT_CALL 54
#define EXIT_CALL 55

#endif
