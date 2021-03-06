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
int QPeek(q_t * p) { return p->q[0]; }

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

    return ret;
}

// if not full, enqueue # to tail slot in queue
void EnQ(int to_add, q_t * p) {
    if(QisFull(p)) {
        // cons_printf("Panic: queue is full, cannot EnQ!\n");
        return;
    }

    p->q[p->tail] = to_add;
    p->tail++;

    return;
}

void MemCpy(char * destination, char * source, int size) {
    int i;
    for(i = 0; i < size; i++) {
        destination[i] = source[i];
    }
}

int StrCmp(char * left, char * right) {
    int i;
    i = 0;

    while(left[i] != '\0' && right[i] != '\0') {
        if(left[i] != right[i]) {
            return FALSE;
        }
        i++;
    }

    if(left[i] != right[i]) {
        return FALSE;
    }

    return TRUE;
}

// We assume buffer size is at least 5
void Itoa(char * str, int x) {
    if(x >= 100000 || x < 0) {
        return;
    }

    str[0] = '0' + x / 10000 % 10;
    str[1] = '0' + x / 1000 % 10;
    str[2] = '0' + x / 100 % 10;
    str[3] = '0' + x / 10 % 10;
    str[4] = '0' + x % 10;
    str[5] = '\0';
}

int AllocatePage(int start) {
    int i;
    for(i = start; i < PAGE_NUM; i++) {
        if(page_user[i] != NONE)
            continue;

        return i;
    }
    
    return NONE;
}
