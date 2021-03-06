#ifndef __K_DATA__
#define __K_DATA__

// Defines PROC_SIZE, PROC_STACK_SIZE
#include "k-const.h"
// Defines q_t, pcb_t
#include "k-type.h"

// PID of running process
extern int run_pid;
extern q_t pid_q, ready_q, sleep_q;
// Process Control Blocks
extern pcb_t pcb[PROC_SIZE];
// process runtime stacks
extern char proc_stack[PROC_SIZE][PROC_STACK_SIZE];
// intr table's DRAM location
extern struct i386_gate * intr_table;
// system time
extern long sys_centi_sec;

// Phase3 data
// kernel has these mutexes to spare
extern mux_t mux[MUX_SIZE]; 
// mutex ID's are initially queued here
extern q_t mux_q;           
// for video access control
extern int vid_mux;         

// Phase4 data
extern term_t term[TERM_SIZE];

#endif
