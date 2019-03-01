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

int MuxCreateCall(int flag) {
    int id;
    asm("movl %1, %%eax;
         int %2;
         movl %%eax, %0"
        : "=g" (id)
        : "g" (flag),
          "g" (MUX_CREATE_CALL)
        : "eax"
    );

    return id;
}

void MuxOpCall(int mux_id, int opcode) {
    asm("movl %0, %%eax;
         movl %1, %%ebx;
         int %2"
        :
        : "g" (mux_id),
          "g" (opcode),
          "g" (MUX_OP_CALL)
        : "eax", "ebx"
    );
}

// composed differently
void WriteCall(int device, char *str) { 
    // get my PID by a service call
    // to set row number
    // column is set zero
    // if device is STDOUT {
    //    while what str points to is not a null character {
    //       Use an existing service call to show this character, at row and column
    //       increment the str pointer and the column position
    //    }
    // }
    int pid, row, col;
    pid = GetPidCall();
    row = pid;
    col = 0;
    while(str) {
        ShowCharCall(row, col++, *str);
        str++;
    }
}
