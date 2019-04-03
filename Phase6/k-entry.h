#ifndef __K_ENTRY__
#define __K_ENTRY__

// Skip beloew if ASSEMBLER defined (from an assembly code)
#ifndef ASSEMBLER

__BEGIN_DECLS

#include "k-type.h"

void TimerEntry(void);
int GetPidEntry(void);
void ShowCharEntry(int, int, char);
void SleepEntry(int);

int MuxCreateEntry(int);
void MuxOpEntry(int, int);

void TERM0Entry(int);
void TERM1Entry(int);

void Loader(trapframe_t *);

// Phase 6

int ForkEntry(void);
int WaitEntry(void);
void ExitEntry(int);

__END_DECLS

#endif

#endif
