#ifndef __K_TYPE__
#define __K_TYPE__

#include "k-const.h"

// Void-return function pointer type
typedef void (*func_p_t)(void);

typedef enum {UNUSED, READY, RUN, SLEEP, SUSPEND} state_t;

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
    // read in 1.html
    state_t state;
    int run_count;
    int total_count;
    // TODO: make long long instead and the operations with it
    int wake_centi_sec;
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
} term_t;

#endif
