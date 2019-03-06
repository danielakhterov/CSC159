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

void swap(int * x, int * y){ 
    int temp = *x; 
    *x = *y; 
    *y = temp; 
}

// get parent index
int parent(int i) { return (i-1)/2; }

// to get index of left child of node at index i 
int left(int i) { return (2*i + 1); } 

// to get index of right child of node at index i 
int right(int i) { return (2*i + 2); } 

void Heapify(q_t * p, int i) { 
    int r, l, smallest;

    l = left(i); 
    r = right(i); 
    smallest = i; 

    if (l < p->tail && p->q[l] < p->q[i]) 
        smallest = l; 
    if (r < p->tail && p->q[r] < p->q[smallest]) 
        smallest = r; 
    if (smallest != i) { 
        swap(&p->q[i], &p->q[smallest]); 
        Heapify(p, smallest); 
    } 
}

void PriorityEnqueue(q_t * p, int pid) {
    int i;
    if(QisFull(p)) {
        cons_printf("Panic: sleep queue is full, cannot EnQ!\n");
        return;
    }

    p->tail++;
    i = p->tail - 1;
    p->q[i] = pid;

    while(i != 0 && p->q[parent(i)] > p->q[i]) {
        swap(&p->q[i], &p->q[parent(i)]);
        i = parent(i);
    }
}

int PriorityDequeue(q_t * p) {
    int root;
    if(QisEmpty(p)) {
        return NONE;
    }

    if (p->tail == 1) { 
        p->tail--; 
        return p->q[0]; 
    } 
  
    // Store the minimum value, and remove it from heap 
    root = p->q[0]; 
    p->q[0] = p->q[p->tail - 1]; 
    p->tail--; 
    Heapify(p, 0); 
  
    return root; 
}
