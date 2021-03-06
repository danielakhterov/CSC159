#include "k-include.h"
#include "k-type.h"
#include "k-data.h"

// Clear DRAM data block, zero-fill it
void Bzero(char * p, int bytes) {
    int i;
    for(i = 0; i < bytes; i++) {
        p[i] = 0;
    }
}

int QisEmpty(q_t * p) { return p->tail == 0; }
int QisFull(q_t * p) { return p->tail == Q_SIZE; }

// Dequeue, 1st # in queue; if queue is empty, return -1 (NONE)
// move rest to front by a notch, set empty spaces -1
int DeQ(q_t * p) {
    int ret, i;

    // return -1 if q[] is empty
    if(QisEmpty(p)) {
        return NONE;
    }

    // return value
    ret = p->q[0];

    // Shift the elements
    for(i = 1; i < p->tail; i++) {
        p->q[i-1] = p->q[i];
    }

    p->tail--;
    p->q[p->tail] = 0;

    return ret;
}

// if not full, enqueue # to tail slot in queue
void EnQ(int to_add, q_t * p) {
    if(QisFull(p)) {
        cons_printf("Panic: queue is full, cannot EnQ!\n");
        return;
    }

    p->q[p->tail] = to_add;
    p->tail++;

    return;
}
