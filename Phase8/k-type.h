#ifndef __K_TYPE__
#define __K_TYPE__

#include "k-const.h"

// Void-return function pointer type
typedef void (* func_p_t)(void);
typedef void (* func_p_t2)(int);

typedef enum {UNUSED, READY, RUN, SLEEP, SUSPEND, WAIT, ZOMBIE, PAUSE} state_t;

typedef struct {
    unsigned int edi;
    unsigned int esi;

    unsigned int ebp;
    unsigned int esp;

    unsigned int ebx;
    unsigned int edx;

    unsigned int ecx;
    unsigned int eax;

    unsigned int entry_id;
    unsigned int eip;
    unsigned int cs;
    unsigned int efl;
} trapframe_t;

typedef struct {
    state_t state;
    int run_count;
    int total_count;
    int wake_centi_sec;
    int ppid;
    int sigint_handler;
    trapframe_t * trapframe_p;
} pcb_t;

typedef struct {
    // generic queue type
    // for a simple queue
    int q[Q_SIZE];
    int tail;
} q_t;

// Phase3
// A mux for IPC
typedef struct {
    // max # of processes to enter
    int flag;

    // requester/owning PID
    int creator;

    // suspended PID's
    q_t suspend_q;
} mux_t;

typedef struct {
    // when initialized or after output last char
    int tx_missed;

    // terminal port I/O base #
    int io_base;

    // flow-control mux
    int out_mux;

    // characters to send to terminal buffered here
    q_t out_q;

    // --------------------------------- //
    //             Phase 5               //
    // --------------------------------- //

    // to buffer terminal KB input
    q_t in_q;

    // to echo back to terminal
    q_t echo_q;

    // to flow-control in_q
    int in_mux;
} term_t;

#endif
