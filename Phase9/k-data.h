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

// Phase 3
extern mux_t mux[MUX_SIZE]; 
extern q_t mux_q;           
extern int vid_mux;         

// Phase 4
extern term_t term[TERM_SIZE];

// Phase 7
extern int page_user[PAGE_NUM];

// Phase 8
extern unsigned rand;

// Phase 9
extern int kernel_main_table;

#endif
