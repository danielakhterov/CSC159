#ifndef __SYS_CALL__
#define __SYS_CALL__

int GetPidCall(void);
void ShowCharCall(int, int, int);
void SleepCall(int);

int MuxCreateCall(int);
void MuxOpCall(int, int);
void WriteCall(int, char *);
void ReadCall(int, char *);

// Phase 6

int ForkCall(void);
int WaitCall(void);
void ExitCall(int);

// Phase 7

void ExecCall(int, int);
void SignalCall(int, int);

// Phase 8

void PauseCall(void);
void KillCall(int, int);
unsigned RandCall(void);

#endif
