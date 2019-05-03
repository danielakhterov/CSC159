#ifndef __K_SR__
#define __K_SR__

void NewProcSR(func_p_t p);
void TimerSR(void);
int GetPidSR(void);
void ShowCharSR(int, int, int);
void SleepSR(int);

int MuxCreateSR(int);
void MuxOpSR(int, int);

void TermSR(int);
void TermTxSR(int);
void TermRxSR(int);

// Phase 6

int ForkSR(void);
int WaitSR(void);
void ExitSR(int);

// Phase 7

void ExecSR(int, int);
void SignalSR(int, int);
void WrapperSR(int, int, int);

// Phase 8

void PauseSR(void);
void KillSR(int, int);
unsigned RandSR(void);

#endif
