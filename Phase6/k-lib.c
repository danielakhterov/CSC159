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
void Itoa(char * str, unsigned x) {
    int i;
    char temp;
    i = 0;

    if(x >= 100000)
        return;

    for(i = 0; i < 5; i++) {
        str[i] = x % 10 + '0';
        x /= 10;
        i++;
    };

    // Reverse string
    temp = str[4];
    str[4] = str[0];
    str[0] = temp;

    temp = str[3];
    str[3] = str[1];
    str[1] = temp;
}
