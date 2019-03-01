#ifndef __K_LIB__
#define __K_LIB__

void Bzero(char *, int);
int QisEmpty(q_t *);
int QisFull(q_t *);

int DeQ(q_t *);
void EnQ(int to_add, q_t *);

int PriorityDequeue(q_t *);
void PriorityEnqueue(q_t *, int);
void Heapify(q_t *, int);
int parent(int);
int left(int);
int right(int);

#endif
