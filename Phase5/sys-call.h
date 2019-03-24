#ifndef __SYS_CALL__
#define __SYS_CALL__

int GetPidCall(void);
void ShowCharCall(int, int, int);
void SleepCall(int);

int MuxCreateCall(int);
void MuxOpCall(int, int);
void WriteCall(int, char *);

#endif
