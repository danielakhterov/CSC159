#ifndef __K_LIB__
#define __K_LIB__

void Bzero(char * p, int bytes);
int QisEmpty(q_t * p);
int QisFull(q_t * p);

int DeQ(q_t * p);
void EnQ(int to_add, q_t * p);

#endif
