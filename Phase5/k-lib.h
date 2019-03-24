#ifndef __K_LIB__
#define __K_LIB__

void Bzero(char *, int);
int QisEmpty(q_t *);
int QisFull(q_t *);

int DeQ(q_t *);
void EnQ(int to_add, q_t *);

#endif
