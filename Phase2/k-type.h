#ifndef __K_TYPE__
#define __K_TYPE__

#include "k-const.h"

// Void-return function pointer type
typedef void (*func_p_t)(void);

typedef enum {UNUSED, READY, RUN, SLEEP} state_t;

typedef struct {
    unsigned int reg[8];
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
    int head;
    int tail;
    int length;
} q_t;

#endif
