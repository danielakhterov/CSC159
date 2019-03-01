// sys-call.c
// calls to kernel services

#include "k-const.h"

int GetPidCall(void) {
    // interrupt!
    // after, copy eax to variable 'pid'
    // output
    // input
    // used registers
    int pid;
    asm("int %1;                
         movl %%eax, %0"         
        : "=g" (pid)           
        : "g" (GETPID_CALL)   
        : "eax"                      
    );

    return pid;
}

void ShowCharCall(int row, int col, int ch) {
    // send in row via eax
    // send in col via ebx
    // send in ch via ecx
    // initiate call, %3 gets entry_id
    asm("movl %0, %%eax;     
         movl %1, %%ebx;     
         movl %2, %%ecx;      
         int %3"            
        :                        
        : "g" (row),       
          "g" (col), 
          "g" (ch),
          "g" (SHOWCHAR_CALL)
        : "eax", "ebx", "ecx"
    );
}

// # of 1/100 of a second to sleep
void SleepCall(int centi_sec) {  
    asm("movl %0, %%eax;
         int %1"
        :
        : "g" (centi_sec),
          "g" (SLEEP_CALL)
        : "eax"
    );
}
