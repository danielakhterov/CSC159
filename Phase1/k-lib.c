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

int QisEmpty(q_t * p) { return p->length == 0; }
int QisFull(q_t * p) { return p->length == Q_SIZE; }

// Dequeue, 1st # in queue; if queue is empty, return -1 (NONE)
// move rest to front by a notch, ste empty spaces -1
int DeQ(q_t * p) {
    // return -1 if q[] is empty
    int ret;
    if(QisEmpty(p)) {
        return NONE;
    }

    ret = p->q[p->head];
    p->q[p->head] = NONE;
    p->head = (p->head + 1) % Q_SIZE;
    p->length--;

    return ret;
}

// if not full, enqueue # to tail slot in queue
void EnQ(int to_add, q_t * p) {
    if(QisFull(p)) {
        cons_printf("Panic: queue is full, cannot EnQ!\n");
        return;
    }

    p->q[p->tail] = to_add;
    p->tail = (p->tail + 1) % Q_SIZE;
    p->length++;

    return;
}
