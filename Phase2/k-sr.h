#ifndef __K_SR__
#define __K_SR__

void NewProcSR(func_p_t p);
void TimerSR(void);
int GetPidSR(void);
void ShowCharSR(int, int, int);
void SleepSR(int);

#endif
