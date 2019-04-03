#ifndef __K_LIB__
#define __K_LIB__

void Bzero(char *, int);
int QisEmpty(q_t *);
int QisFull(q_t *);

int DeQ(q_t *);
void EnQ(int, q_t *);

// Phase 6

void MemCpy(char *, char *, int);
int StrCmp(char *, char *);
void Itoa(char *, unsigned);

#endif
